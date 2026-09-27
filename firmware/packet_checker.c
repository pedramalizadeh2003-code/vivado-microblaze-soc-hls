#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define HEADER 0x3b
#define COMMREAD 0xff
#define COMMWRITE 0x00

// Global variables
int count_packets;
int count_response;
uint16_t checksum;
uint16_t tmp_checksum = 0x0000;
uint8_t outdata0, outdata1, outdata2, outdata3;
uint8_t checksumH, checksumL;

// Defining the states for packet processing
typedef enum
{
    ND,
    HDR,
    CMD,
    WR,
    RD
} PacketState;
typedef enum
{
    IDLE,
    COMMAND,
    DATA,
    CHKSUM
} ResponseState;

// Defining a struct to return multiple values from the function
struct Output_signals
{
    uint16_t wrAddress;
    uint32_t wrData;
    bool wrEn;
    uint16_t rdAddress;
    bool rdEn;
    bool error;
    uint8_t dataOut;
    bool dataOutRdy;
    PacketState Packet_state;
    ResponseState Response_state;
};

// Defining a global struct object to keep values
struct Output_signals out;

// Function Definition
struct Output_signals PacketProcess(bool clk, bool rst, uint8_t dataIn, bool dataInRdy, uint32_t rdData, bool rdRdy);

int main()
{
    uint8_t InputData1[12] = {0x00, 0x3b, 0x00, 0x06, 0x14, 0xff, 0x55, 0x01, 0x21, 0x01, 0xcb, 0x00};
    uint8_t InputData2[8] = {0x00, 0x3b, 0xff, 0x06, 0x14, 0x01, 0x54, 0x00};
    bool reset = false;
    bool clock = true;
    bool InputReady = true;
    uint32_t ReadData = 0x12345678;
    bool ReadReady = false;
    struct Output_signals out3;

    for (int i = 0; i < 30; i++)
    {
        if (i < 12) //* write command test *//
        {
            out3 = PacketProcess(clock, reset, InputData1[i], InputReady, ReadData, ReadReady);
            printf("\n");
            printf("state : %d\n", out3.Packet_state);
            printf("count packets : %d\n", count_packets);
            printf("wrAddress: %x\n", out3.wrAddress);
            printf("wrData: %x\n", out3.wrData);
            printf("checksum: %x\n", ((uint16_t)checksumH << 8) + (uint16_t)checksumL);
            printf("cal_checksum: %x\n", tmp_checksum);
            printf("WrEn: %d\n", out3.wrEn);
        }
        else if (i >= 12 && i < 20) //* read command test *//
        {
            out3 = PacketProcess(clock, reset, InputData2[i - 12], InputReady, ReadData, ReadReady);
            printf("\n");
            printf("state : %d\n", out3.Packet_state);
            printf("count packets : %d\n", count_packets);
            printf("rdAddress: %x\n", out3.rdAddress);
            printf("rdEn: %d\n", out3.rdEn);
            printf("checksum: %x\n", ((uint16_t)checksumH << 8) + (uint16_t)checksumL);
            printf("cal_checksum: %x\n", tmp_checksum);
        }
        else if (i >= 20) //* read data test *//
        {
            if (i == 21)
            {
                ReadReady = true;
            }
            else
            {
                ReadReady = false;
            }
            out3 = PacketProcess(clock, reset, 0x00, InputReady, ReadData, ReadReady);
            printf("\n");
            printf("states : %d , %d\n", out3.Packet_state, out3.Response_state);
            printf("rdRdy : %d\n", ReadReady);
            printf("count rsponse : %d\n", count_response);
            printf("dataOut : %x\n", out3.dataOut);
            printf("dataOutRdy : %d\n", out3.dataOutRdy);
        }
    }

    return 0;
}

// Packet Processor Definition
struct Output_signals PacketProcess(bool clk, bool rst, uint8_t dataIn, bool dataInRdy, uint32_t rdData, bool rdRdy)
{

    struct Output_signals out2;

    if (clk)
    {

        out.error = false;
        out.wrEn = false;
        out.rdEn = false;
        out.dataOutRdy = false;

        if (rst)
        {
            //*** TODO ***//
            // Reset all signals to zero
            count_packets = 0;
            count_response = 0;
            out.Packet_state = ND;
            out.Response_state = IDLE;
        }
        else
        {
            // Check if it's the right time to look for the DataIn
            if (dataInRdy)
            {

                // Check which packet we should look for
                switch (out.Packet_state)
                {

                // Header is not recieved yet
                case ND:
                    // Look for header
                    //*** TODO ***//
                    tmp_checksum = 0;
                    count_packets = 0;
                    if (dataIn == HEADER)
                    {
                        out.Packet_state = HDR;
                        tmp_checksum += dataIn;
                    }

                    break;

                // Header is Seen!
                case HDR:

                    // Look for Command
                    //*** TODO ***//
                    if (dataIn == COMMWRITE)
                    {
                        out.Packet_state = WR;
                        tmp_checksum += dataIn;
                    }
                    else if (dataIn == COMMREAD)
                    {
                        out.Packet_state = RD;
                        tmp_checksum += dataIn;
                    }
                    else
                    {
                        out.error = true;
                        printf("the command is not valid !");
                    }

                    break;

                // Command is Seen!
                case CMD:

                    // Check if command is for WRITE or READ
                    //*** TODO ***//

                    break;

                // Write Command is Seen!
                case WR:

                    //*** TODO ***//

                    switch (count_packets)
                    {
                    case 0:
                        out.wrAddress = dataIn;
                        out.wrAddress = out.wrAddress << 8;
                        tmp_checksum += dataIn;
                        count_packets += 1;
                        break;

                    case 1:
                        out.wrAddress += dataIn;
                        tmp_checksum += dataIn;
                        count_packets += 1;
                        break;

                    case 2:
                        out.wrData += (uint32_t)dataIn << 24;
                        tmp_checksum += dataIn;
                        count_packets += 1;
                        break;

                    case 3:
                        out.wrData += (uint32_t)dataIn << 16;
                        tmp_checksum += dataIn;
                        count_packets += 1;
                        break;

                    case 4:
                        out.wrData += (uint32_t)dataIn << 8;
                        tmp_checksum += dataIn;
                        count_packets += 1;
                        break;

                    case 5:
                        out.wrData += (uint32_t)dataIn;
                        tmp_checksum += dataIn;
                        count_packets += 1;
                        break;

                    case 6:
                        checksumH = dataIn;
                        count_packets += 1;
                        break;

                    case 7:
                        checksumL = dataIn;
                        count_packets += 1;
                        break;

                    case 8:
                        out.Packet_state = ND;

                        if (tmp_checksum == (((uint16_t)checksumH << 8) + (uint16_t)checksumL))
                        {
                            out.wrEn = true;
                        }
                        else
                        {
                            out.error = true;
                            printf("CheckSum error !");
                        }
                        break;
                    }

                    break;

                // Read Command is seen!
                case RD:

                    //*** TODO ***//

                    switch (count_packets)
                    {
                    case 0:
                        out.rdAddress = dataIn;
                        out.rdAddress = out.rdAddress << 8;
                        tmp_checksum += dataIn;
                        count_packets++;
                        break;

                    case 1:
                        out.rdAddress += dataIn;
                        tmp_checksum += dataIn;
                        count_packets++;
                        break;

                    case 2:
                        checksumH = dataIn;
                        count_packets++;
                        break;

                    case 3:
                        checksumL = dataIn;
                        count_packets++;
                        break;

                    case 4:
                        out.Packet_state = ND;

                        if (tmp_checksum == (((uint16_t)checksumH << 8) + (uint16_t)checksumL))
                        {
                            out.rdEn = true;
                        }
                        else
                        {
                            out.error = true;
                            printf("CheckSum error !");
                        }
                        break;
                    }
                    break;
                }
            }
            switch (out.Response_state)
            {

            case IDLE:

                checksum = 0x00;
                count_response = 0;
                if (rdRdy == true)
                {
                    // Initialize sending packets in given order
                    //*** TODO ***//
                    out.Response_state = COMMAND;
                    out.dataOut = HEADER;
                    out.dataOutRdy = true;
                    checksum += HEADER;
                }
                break;

            case COMMAND:
                // Send Command packet and get data packets ready to be sent
                //*** TODO ***//
                out.Response_state = DATA;
                out.dataOut = COMMREAD;
                out.dataOutRdy = true;
                checksum += COMMREAD;
                break;

            case DATA:
                // Send Data packets and Calculate CheckSum
                //*** TODO ***//
                switch (count_response)
                {
                case 0:
                    out.dataOut = (rdData >> 24) & 0xFF;
                    out.dataOutRdy = true;
                    checksum += (rdData >> 24) & 0xFF;
                    count_response++;
                    break;

                case 1:
                    out.dataOut = (rdData >> 16) & 0xFF;
                    out.dataOutRdy = true;
                    checksum += (rdData >> 16) & 0xFF;
                    count_response++;
                    break;

                case 2:
                    out.dataOut = (rdData >> 8) & 0xFF;
                    out.dataOutRdy = true;
                    checksum += (rdData >> 8) & 0xFF;
                    count_response++;
                    break;

                case 3:
                    out.dataOut = rdData & 0xFF;
                    out.dataOutRdy = true;
                    checksum += rdData & 0xFF;
                    out.Response_state = CHKSUM;
                    count_response++;
                    break;
                }
                break;

            case CHKSUM:
                // Send CheckSum packets and reset variables
                //*** TODO ***//
                switch (count_response)
                {
                case 4:
                    out.dataOut = (checksum >> 8) & 0xFF;
                    out.dataOutRdy = true;
                    count_response++;
                    break;

                case 5:
                    out.dataOut = checksum & 0xFF;
                    out.dataOutRdy = true;
                    out.Response_state = IDLE;
                    break;
                }
                break;
            }
        }
    }

    out2 = out;
    return out2;
}
