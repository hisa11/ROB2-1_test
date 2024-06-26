#include "mbed.h"
#include "firstpenguin.hpp"

constexpr uint32_t can_id = 35;
int suuti = 0;
int gohan = 0;
int tarn = 0;
BufferedSerial pc(USBTX, USBRX, 250000); // パソコンとのシリアル通信
Timer timer;
CAN can(PA_11, PA_12, (int)1e6);
CAN can1(PB_12, PB_13, (int)1e6);
FirstPenguin penguin{can_id, can1};
uint8_t DATA[8] = {};

void readUntilPipe(char *output_buf, int output_buf_size)
{
    char buf[20];
    int output_buf_index = 0;
    while (1)
    {
        if (pc.readable())
        {
            ssize_t num = pc.read(buf, sizeof(buf) - 1); // -1 to leave space for null terminator
            buf[num] = '\0';
            for (int i = 0; i < num; i++)
            {
                if (buf[i] == '|')
                {
                    output_buf[output_buf_index] = '\0';
                    return;
                }
                else if (buf[i] != '\n' && output_buf_index < output_buf_size - 1)
                { // 改行文字を無視します
                    output_buf[output_buf_index++] = buf[i];
                }
            }
        }
        if (output_buf_index >= output_buf_size - 1) // Prevent buffer overflow
        {
            output_buf[output_buf_index] = '\0';
            return;
        }
    }
}

void can_send()
{
    while (1)
    {
        DATA[0] = suuti + tarn >> 8; // ビッグエンディアン形式
        DATA[1] = suuti + tarn & 0xFF;
        DATA[2] = -suuti + tarn >> 8; // ビッグエンディアン形式
        DATA[3] = -suuti + tarn & 0xFF;
        CANMessage msg0(0x200, DATA, 8);
        can.write(msg0);
        penguin.send();
        // printf("pwm[2]: %d\n", penguin.pwm[2]);
        ThisThread::sleep_for(10ms); // 100ms待機
    }
}

int main()
{
    char output_buf[20]; // 出力用のバッファを作成します
    Thread thread;
    thread.start(can_send); // can_sendスレッドを開始

    while (1)
    {
        readUntilPipe(output_buf, sizeof(output_buf)); // '|'が受け取られるまでデータを読み込みます
        // printf("%s\n", output_buf);
        if (strncmp(output_buf, "x_send", 6) == 0)
        {
            suuti = -2000;
            // gohan = 10000; // 修正: == から = へ
            printf("cross\n");
        }
        else if (strncmp(output_buf, "un_x", 4) == 0 or strncmp(output_buf, "un_triangle", 11) == 0)
        {
            suuti = 0;
            // gohan = 0; // 修正: == から = へ
            printf("un_cross\n");
        }
        else if (strncmp(output_buf, "triangle", 8) == 0)
        {
            suuti = 2000;
            printf("triangle\n");
        }
        else if (strncmp(output_buf, "right", 2) == 0)
        {
            gohan = 3000;
            printf("right\n");
        }
        else if (strncmp(output_buf, "left", 4) == 0)
        {
            gohan = -3000;
            printf("left\n");
        }
        else if (strncmp(output_buf, "un_arrow", 8) == 0)
        {
            gohan = 0;
            printf("un_arrow\n");
        }
        else if (strncmp(output_buf, "R1", 2) == 0)
        {
            tarn = 1000;
            printf("R1\n");
        }
        else if (strncmp(output_buf, "un_R1", 5) == 0 or strncmp(output_buf, "un_L1", 5) == 0)
        {
            tarn = 0;
            printf("un_R1\n");
        }
        else if (strncmp(output_buf, "L1", 2) == 0)
        {
            tarn = -1000;
            printf("L1\n");
        }

        penguin.pwm[0] = -gohan;
        penguin.pwm[1] = -gohan;
        penguin.pwm[2] = -gohan;
        penguin.pwm[3] = -gohan;
        printf("%d\n", suuti + tarn); // 修正された行
    }
}
