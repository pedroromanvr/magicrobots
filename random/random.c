
#include "random.h"

uint16_t randomIdx = 0;
eeprom uint8_t numbers[1024] = {
0xF0, 254, 0, 210, 238, 166, 60, 106, 87, 210, 2, 191, 174, 47, 247, 100,
212, 144, 211, 166, 230, 251, 190, 184, 210, 178, 14, 15, 184, 51, 148, 23,
102, 132, 143, 206, 150, 17, 52, 50, 40, 147, 252, 3, 233, 85, 166, 206,
159, 136, 145, 109, 234, 48, 105, 241, 183, 204, 61, 178, 253, 2, 6, 167,
164, 216, 203, 124, 179, 241, 254, 150, 91, 98, 177, 221, 24, 72, 184, 58,
111, 48, 197, 135, 104, 78, 159, 187, 51, 201, 7, 70, 113, 17, 203, 115,
233, 191, 241, 192, 142, 99, 34, 14, 87, 87, 16, 101, 123, 87, 188, 151,
20, 4, 165, 173, 219, 197, 249, 153, 105, 13, 161, 3, 155, 94, 251, 190,
233, 138, 136, 82, 39, 81, 184, 163, 39, 127, 200, 30, 248, 147, 223, 113,
129, 179, 236, 249, 7, 23, 94, 104, 238, 144, 249, 197, 104, 201, 208, 108,
123, 199, 80, 207, 87, 152, 48, 79, 217, 153, 69, 15, 161, 192, 118, 166,
170, 149, 75, 209, 15, 193, 76, 250, 196, 40, 6, 200, 113, 15, 145, 130,
123, 236, 93, 191, 52, 221, 80, 169, 25, 112, 242, 163, 247, 78, 0, 94,
67, 240, 204, 44, 129, 177, 81, 53, 221, 207, 240, 47, 57, 131, 50, 19,
217, 186, 152, 229, 32, 194, 98, 76, 124, 241, 135, 138, 27, 118, 102, 99,
13, 47, 122, 95, 55, 151, 21, 69, 64, 93, 28, 164, 233, 52, 116, 132,
70, 8, 242, 186, 81, 87, 83, 223, 0, 89, 116, 190, 190, 155, 163, 223,
159, 137, 209, 96, 70, 110, 75, 211, 36, 57, 90, 117, 54, 230, 189, 97,
29, 210, 158, 146, 157, 208, 180, 60, 142, 177, 194, 129, 174, 201, 92, 217,
237, 13, 186, 159, 61, 189, 210, 74, 76, 39, 200, 47, 72, 123, 226, 35,
211, 5, 36, 157, 84, 232, 184, 53, 13, 144, 206, 229, 4, 232, 128, 208,
182, 101, 119, 150, 122, 175, 214, 213, 147, 160, 107, 61, 192, 44, 148, 212,
197, 142, 82, 38, 18, 225, 53, 193, 152, 203, 105, 22, 136, 191, 148, 168,
213, 215, 92, 105, 226, 209, 135, 244, 109, 63, 41, 189, 99, 117, 224, 121,
82, 33, 239, 234, 185, 51, 243, 45, 101, 232, 151, 151, 42, 228, 248, 115,
235, 23, 69, 42, 205, 115, 77, 184, 208, 234, 21, 181, 80, 70, 247, 64,
193, 232, 31, 236, 106, 96, 152, 156, 143, 64, 221, 227, 47, 162, 135, 156,
167, 151, 23, 109, 115, 226, 236, 240, 192, 198, 209, 6, 138, 185, 149, 247,
172, 95, 128, 238, 227, 145, 215, 14, 222, 140, 210, 181, 198, 233, 185, 160,
66, 86, 23, 35, 203, 102, 6, 161, 67, 255, 244, 213, 105, 112, 84, 88,
138, 227, 49, 255, 208, 244, 65, 30, 130, 171, 165, 108, 237, 76, 114, 241,
112, 159, 230, 34, 92, 136, 32, 117, 31, 26, 147, 102, 6, 48, 67, 4,
62, 217, 122, 103, 92, 181, 69, 103, 2, 97, 67, 205, 33, 70, 165, 161,
63, 240, 75, 7, 12, 170, 151, 197, 28, 227, 254, 183, 129, 216, 191, 5,
146, 100, 99, 117, 100, 239, 55, 74, 128, 71, 172, 222, 165, 66, 16, 115,
71, 142, 61, 95, 206, 11, 249, 237, 65, 186, 14, 91, 133, 71, 243, 85,
210, 223, 202, 137, 38, 162, 98, 241, 221, 163, 239, 90, 85, 12, 217, 127,
183, 206, 250, 88, 2, 120, 37, 228, 98, 194, 158, 122, 32, 112, 205, 43,
7, 212, 118, 248, 211, 54, 194, 50, 95, 101, 125, 24, 76, 35, 242, 222,
241, 90, 45, 191, 204, 28, 135, 77, 22, 45, 121, 38, 20, 124, 252, 59,
129, 177, 67, 168, 117, 203, 155, 177, 218, 31, 218, 126, 229, 44, 138, 134,
75, 230, 1, 210, 189, 3, 94, 235, 223, 117, 105, 49, 144, 195, 211, 179,
126, 95, 57, 215, 246, 90, 14, 160, 54, 61, 163, 118, 28, 9, 109, 55,
95, 151, 75, 54, 239, 39, 94, 84, 143, 246, 251, 14, 42, 226, 15, 69,
29, 92, 164, 186, 168, 19, 244, 238, 187, 223, 192, 167, 254, 42, 238, 56,
235, 116, 15, 75, 241, 236, 244, 127, 60, 249, 153, 23, 113, 164, 133, 91,
140, 82, 112, 1, 227, 171, 62, 216, 167, 189, 137, 201, 71, 204, 236, 13,
235, 72, 123, 201, 239, 218, 162, 210, 21, 89, 146, 191, 192, 58, 186, 30,
199, 124, 119, 69, 242, 77, 195, 96, 24, 86, 45, 101, 105, 140, 21, 109,
46, 109, 122, 75, 83, 53, 9, 123, 182, 225, 142, 112, 191, 121, 199, 162,
249, 123, 70, 162, 190, 116, 167, 178, 144, 163, 167, 198, 173, 81, 235, 90,
99, 222, 143, 60, 53, 1, 92, 186, 159, 183, 180, 223, 58, 32, 215, 79,
82, 39, 81, 62, 179, 81, 86, 213, 121, 174, 134, 254, 172, 74, 233, 213,
16, 178, 157, 239, 57, 28, 206, 188, 143, 195, 7, 96, 96, 126, 199, 245,
196, 50, 26, 153, 114, 179, 90, 180, 58, 97, 215, 195, 250, 56, 147, 180,
124, 79, 188, 70, 105, 228, 189, 205, 156, 159, 184, 17, 34, 158, 185, 220,
249, 194, 19, 93, 132, 159, 249, 121, 160, 255, 97, 98, 109, 217, 118, 163,
68, 43, 157, 144, 52, 90, 153, 58, 163, 244, 64, 48, 106, 126, 167, 86,
65, 220, 227, 11, 156, 47, 109, 42, 145, 14, 213, 180, 214, 34, 45, 214,
144, 15, 238, 212, 150, 14, 45, 89, 226, 141, 58, 28, 184, 210, 42, 123,
6, 43, 61, 140, 65, 147, 218, 245, 156, 30, 223, 212, 231, 194, 82, 4,
98, 251, 85, 108, 176, 61, 246, 243, 239, 213, 213, 241, 55, 251, 16, 6,
157, 189, 158, 230, 151, 67, 183, 56, 205, 26, 207, 161, 213, 97, 127, 111,
72, 129, 206, 45, 38, 41, 38, 123, 169, 137, 235, 49, 202, 152, 51, 77};
uint16_t random16()
{
    uint16_t ret = random8();
    ret |= (((uint16_t)random8())<<8);
    return ret;
}
