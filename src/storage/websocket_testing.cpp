#include <iostream>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

// Decode WebSocket message
vector<char> decodeWebSocketMessage(const vector<char>& encodedMsg)
{
    vector<char> decodedMsg;

    // First byte
    char opcode = encodedMsg[0] & 15;
    bool fin = (encodedMsg[0] & 128) != 0;
    bool mask = (encodedMsg[1] & 128) != 0;
    uint64_t payloadLength = encodedMsg[1] & 127;
    int payloadOffset = 2;

    // Extended payload length
    if (payloadLength == 126) {
        payloadLength = (encodedMsg[2] << 8) + encodedMsg[3];
        payloadOffset = 4;
    }
    else if (payloadLength == 127) {
        payloadLength = ((uint64_t)encodedMsg[2] << 56) + ((uint64_t)encodedMsg[3] << 48) +
            ((uint64_t)encodedMsg[4] << 40) + ((uint64_t)encodedMsg[5] << 32) +
            ((uint64_t)encodedMsg[6] << 24) + ((uint64_t)encodedMsg[7] << 16) +
            ((uint64_t)encodedMsg[8] << 8) + encodedMsg[9];
        payloadOffset = 10;
    }

    // Mask key
    vector<char> maskKey;
    if (mask) {
        for (int i = 0; i < 4; ++i) {
            maskKey.push_back(encodedMsg[payloadOffset + i]);
        }
        payloadOffset += 4;
    }

    // Payload data
    for (uint64_t i = 0; i < payloadLength; ++i) {
        char c = encodedMsg[payloadOffset + i];
        if (mask) {
            c = c ^ maskKey[i % 4];
        }
        decodedMsg.push_back(c);
    }

    return decodedMsg;
}

int main()
{
    // Example encoded message
    vector<char> encodedMsg = { 129, 133, 55, 250, 33, 61, 127, 159, 77, 81 };
    // Decoding message
    vector<char> decodedMsg = decodeWebSocketMessage(encodedMsg);
    // Printing the result
    for (char c : decodedMsg) {
        cout << c;
    }
    cout << endl;

    return 0;
}