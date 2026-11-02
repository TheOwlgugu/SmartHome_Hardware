#include "esp8266.h"
#include "uart.h"
#include "delay.h"

/* 解析状态：收到小写 'i' 后置1，等下一个字符是 '0' / '1' / '2' */
static bit flag_rx_i = 0;

/* AT 配置期间置0，不解析，避免把 ESP 的应答当成手机指令 */
static bit esp_ready = 1;

/* 裸收发模式：直接发 "sXX\r\n"，不套 AT+CIPSEND。
   没调用 ESP_ATSetup() 时就是这个模式，方便用电脑串口助手调试硬件。
   调用 ESP_ATSetup() 之后自动切成 CIPSEND 模式。                  */
static bit raw_mode = 1;

/* 手机下发的待处理指令，主循环取走。0 = 无 */
static u8 pending_cmd = 0;

/* ---- 链路号跟踪 ----
   ESP 的 TCP Server 给每个连接分配 0~4 的编号，数据以 +IPD,<id>,<len>: 开头。
   手机掉线重连后编号可能变，所以不能把 AT+CIPSEND 的链路号写死成 0，
   否则会往一条已经死掉的链路发，手机再也收不到状态。
   这里把 "+IPD," 后面那个数字抓出来，回复时原样用它。              */
static u8 code IPD_STR[] = "+IPD,";
static u8 ipd_pos = 0;          /* "+IPD," 的匹配进度 */
static bit expect_link_id = 0;  /* 下一个字符就是链路号 */
static u8 link_id = '0';        /* 以 ASCII 存，直接发出去 */

void ESP_Init() {
    UART_RxCallback = ESP_ReceiveHandler;
    esp_ready = 1;
    raw_mode  = 1;
    link_id   = '0';
}

/* 开机配置 ESP8266，把它变成 TCP Server，手机 App 作为 Client 连过来。

   为什么必须每次开机都发：
   AT 1.7.4 里 CWMODE / CWJAP 是写 flash 的，断电不丢；
   但 CIPMUX 和 CIPSERVER 不存 flash，每次上电都会复位，
   所以这两条只能由 51 每次开机重新发一遍。                  */
void ESP_ATSetup() {
    esp_ready = 0;      /* 配置期间屏蔽解析 */
    raw_mode  = 1;      /* AT 指令本身要裸发 */

    Delay(3000);                                /* 等 ESP 上电自检完成 */

    UART_SendString("AT\r\n");
    Delay(500);

    UART_SendString("ATE0\r\n");                /* 关回显，减少串口噪音 */
    Delay(500);

#ifdef ESP_CONFIG_WIFI
    UART_SendString("AT+CWMODE=1\r\n");         /* Station 模式 */
    Delay(1000);

    UART_SendString("AT+CWJAP=\"" ESP_WIFI_SSID "\",\"" ESP_WIFI_PASS "\"\r\n");
    Delay(10000);                               /* 连 WiFi 慢，给足时间 */
#else
    Delay(6000);        /* 不自己配 WiFi，留时间给 ESP 用 flash 里的配置自动重连 */
#endif

    UART_SendString("AT+CIPMUX=1\r\n");         /* 开 Server 必须先开多连接 */
    Delay(500);

    UART_SendString("AT+CIPSERVER=1," ESP_TCP_PORT "\r\n");
    Delay(1000);

    raw_mode  = 0;      /* 之后上报都要套 AT+CIPSEND */
    esp_ready = 1;
}

/* 上报格式：s + 灯状态 + 风扇状态，例如 s10

   多连接模式下不能透传，每次要先发 AT+CIPSEND=<链路号>,3。
   长度恒为 3，所以只有链路号是变的，用固定延时代替解析 '>' 提示符即可。 */
void ESP_SendStatus(u8 light, u8 fan) {
    char buf[6];

    buf[0] = 's';
    buf[1] = light ? '1' : '0';
    buf[2] = fan   ? '1' : '0';

    if (raw_mode) {
        /* 调试模式：补上换行，电脑串口助手里一行一条看得清楚 */
        buf[3] = '\r';
        buf[4] = '\n';
        buf[5] = '\0';
        UART_SendString(buf);
        return;
    }

    buf[3] = '\0';
    UART_SendString("AT+CIPSEND=");
    UART_SendByte(link_id);
    UART_SendString(",3\r\n");
    Delay(100);          /* 等 ESP 回 '>' */
    UART_SendString(buf);
    Delay(100);          /* 等 SEND OK */
}

/* 串口中断里调用，只做最小的事：识别指令并存起来，绝不在这里发串口。

   手机发来的 i1 / i2 在串口上实际长这样：+IPD,0,2:i1
   先用这个前缀抓链路号，剩下的 i<数字> 照常解析。

   i0 是手机每 5 秒发一次的心跳兼查询包：不改任何状态，只回一条 sXX。 */
void ESP_ReceiveHandler(u8 rx_data) {
    if (!esp_ready) {
        return;
    }

    /* ---- 先抓链路号 ---- */
    if (expect_link_id) {
        expect_link_id = 0;
        if (rx_data >= '0' && rx_data <= '4') {
            link_id = rx_data;
        }
        return;
    }

    if (rx_data == IPD_STR[ipd_pos]) {
        ipd_pos++;
        if (IPD_STR[ipd_pos] == '\0') {     /* "+IPD," 整个匹配上了 */
            ipd_pos = 0;
            expect_link_id = 1;
        }
        return;     /* '+' 'I' 'P' 'D' ',' 都不是 'i'，直接返回不影响下面的解析 */
    }
    ipd_pos = (rx_data == '+') ? 1 : 0;

    /* ---- 再解析 i<数字> ---- */
    if (flag_rx_i) {
        flag_rx_i = 0;
        if (rx_data == '1') {
            pending_cmd = 1;        /* 控制灯 */
            return;
        } else if (rx_data == '2') {
            pending_cmd = 2;        /* 控制风扇 */
            return;
        } else if (rx_data == '0') {
            pending_cmd = 3;        /* 心跳/查询，只上报 */
            return;
        }
        /* 不是数字就当没看见，继续往下判断这个字符本身是不是 'i' */
    }

    if (rx_data == 'i') {
        flag_rx_i = 1;
    }
}

/* 主循环调用。关中断取值，避免正好被串口中断改掉 */
u8 ESP_GetCommand() {
    u8 cmd;

    ES = 0;
    cmd = pending_cmd;
    pending_cmd = 0;
    ES = 1;

    return cmd;
}
