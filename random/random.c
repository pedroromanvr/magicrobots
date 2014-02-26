
#include "random.h"

uint16_t randomIdx = 0;

eeprom uint8_t numbers[1024] =
{27, 100, 149, 139, 88, 121, 57, 187, 65, 247, 143, 97, 42, 164, 37, 244,
91, 200, 29, 252, 152, 144, 95, 28, 33, 173, 107, 62, 74, 179, 212, 117,
155, 109, 225, 181, 171, 118, 156, 13, 111, 22, 49, 29, 6, 28, 156, 40,
94, 232, 207, 200, 191, 11, 211, 123, 239, 221, 172, 133, 115, 35, 63, 72,
119, 114, 51, 252, 200, 222, 46, 63, 61, 240, 3, 172, 61, 30, 240, 10,
171, 208, 243, 252, 234, 87, 237, 152, 7, 153, 120, 148, 130, 36, 173, 21,
110, 76, 196, 185, 204, 85, 189, 180, 202, 231, 0, 199, 178, 115, 241, 155,
122, 78, 218, 192, 42, 185, 214, 93, 201, 13, 149, 224, 187, 116, 56, 20,
115, 80, 178, 69, 150, 149, 45, 6, 62, 205, 49, 123, 28, 31, 139, 30,
104, 145, 116, 143, 129, 33, 144, 29, 253, 190, 210, 36, 176, 114, 125, 154,
206, 158, 99, 10, 35, 146, 57, 26, 5, 119, 141, 74, 250, 85, 67, 136,
180, 153, 162, 185, 17, 49, 178, 16, 174, 104, 195, 11, 80, 128, 57, 20,
103, 118, 154, 168, 234, 180, 14, 42, 167, 207, 202, 205, 206, 119, 213, 171,
76, 253, 21, 134, 85, 76, 124, 53, 172, 105, 36, 74, 12, 55, 164, 199,
190, 51, 76, 31, 248, 143, 167, 25, 179, 210, 70, 74, 211, 105, 185, 242,
228, 182, 180, 125, 84, 55, 174, 216, 228, 201, 78, 230, 153, 123, 243, 132,
69, 114, 147, 139, 239, 110, 58, 183, 35, 211, 116, 33, 20, 24, 255, 207,
86, 95, 210, 207, 93, 156, 70, 92, 89, 42, 222, 89, 58, 156, 17, 136,
214, 221, 210, 121, 10, 89, 169, 88, 43, 209, 22, 215, 50, 232, 153, 199,
143, 132, 136, 84, 105, 5, 108, 153, 181, 190, 108, 168, 180, 131, 127, 198,
40, 49, 90, 164, 248, 138, 193, 206, 99, 30, 211, 221, 100, 223, 195, 121,
149, 69, 153, 96, 65, 26, 250, 97, 165, 92, 222, 86, 201, 119, 94, 242,
218, 148, 0, 139, 157, 56, 49, 130, 237, 33, 1, 243, 56, 214, 198, 66,
74, 116, 5, 124, 235, 240, 114, 1, 12, 195, 172, 48, 168, 252, 55, 21,
227, 71, 176, 92, 71, 108, 217, 167, 48, 93, 52, 234, 30, 108, 192, 23,
44, 32, 141, 66, 55, 50, 7, 223, 138, 133, 112, 69, 204, 211, 187, 119,
191, 233, 135, 211, 119, 211, 108, 211, 110, 164, 222, 34, 157, 144, 155, 49,
83, 152, 106, 93, 140, 240, 239, 241, 145, 201, 38, 158, 133, 52, 210, 221,
11, 198, 121, 93, 53, 238, 223, 2, 243, 92, 179, 236, 49, 113, 59, 96,
125, 61, 53, 220, 153, 85, 95, 134, 169, 113, 250, 42, 70, 9, 211, 166,
105, 117, 100, 225, 77, 228, 85, 111, 63, 80, 102, 243, 159, 252, 42, 78,
234, 92, 151, 156, 209, 32, 144, 6, 83, 132, 232, 206, 93, 197, 176, 249,
24, 32, 236, 48, 31, 119, 96, 204, 235, 214, 102, 104, 26, 59, 224, 202,
50, 253, 89, 73, 207, 43, 137, 127, 94, 141, 184, 246, 21, 251, 254, 235,
44, 19, 11, 34, 253, 28, 161, 170, 33, 46, 248, 117, 101, 205, 216, 53,
28, 178, 238, 3, 35, 45, 0, 81, 145, 58, 182, 150, 192, 64, 103, 87,
109, 94, 128, 62, 207, 242, 239, 125, 218, 244, 132, 144, 53, 38, 99, 52,
128, 102, 214, 237, 37, 109, 6, 76, 254, 116, 241, 203, 201, 101, 128, 43,
134, 211, 244, 19, 7, 116, 207, 218, 108, 207, 108, 130, 230, 79, 114, 97,
57, 104, 27, 87, 124, 132, 233, 28, 115, 188, 68, 93, 133, 54, 46, 37,
12, 165, 11, 79, 186, 135, 16, 33, 84, 207, 0, 132, 58, 74, 197, 84,
43, 33, 61, 247, 67, 237, 234, 170, 68, 82, 218, 128, 114, 176, 10, 124,
1, 24, 22, 198, 111, 42, 111, 242, 27, 117, 78, 92, 245, 133, 134, 29,
231, 166, 25, 105, 208, 91, 108, 1, 26, 87, 69, 87, 169, 132, 87, 14,
147, 22, 212, 222, 79, 60, 95, 71, 49, 41, 65, 135, 10, 91, 199, 114,
19, 243, 38, 20, 229, 152, 167, 225, 4, 59, 135, 134, 100, 136, 0, 144,
110, 1, 4, 69, 30, 237, 99, 95, 3, 181, 181, 150, 132, 116, 15, 34,
165, 249, 247, 37, 77, 199, 215, 199, 172, 216, 14, 151, 203, 145, 65, 172,
235, 249, 10, 130, 24, 69, 235, 198, 156, 5, 242, 224, 124, 118, 187, 247,
163, 166, 33, 188, 46, 180, 134, 73, 11, 69, 122, 110, 188, 169, 84, 56,
167, 152, 109, 166, 168, 90, 62, 28, 9, 130, 46, 119, 16, 74, 211, 71,
183, 127, 168, 191, 214, 138, 21, 247, 74, 171, 4, 118, 219, 45, 108, 98,
13, 162, 133, 36, 50, 48, 220, 76, 116, 34, 176, 245, 4, 198, 21, 180,
185, 188, 238, 114, 141, 96, 21, 209, 128, 86, 144, 105, 206, 237, 234, 77,
161, 203, 27, 79, 102, 13, 94, 158, 145, 5, 106, 166, 4, 186, 168, 60,
160, 104, 124, 223, 32, 152, 119, 161, 112, 66, 162, 40, 129, 124, 38, 229,
86, 249, 77, 75, 168, 89, 191, 62, 104, 180, 165, 139, 244, 140, 160, 40,
73, 193, 101, 120, 226, 221, 206, 10, 26, 36, 214, 96, 57, 96, 32, 160,
111, 181, 236, 118, 229, 23, 19, 7, 253, 57, 163, 49, 159, 147, 121, 103,
126, 227, 158, 92, 250, 95, 65, 178, 165, 212, 246, 0, 214, 202, 11, 60,
9, 85, 243, 7, 170, 242, 132, 137, 167, 50, 125, 119, 16, 21, 131, 222,
38, 74, 194, 34, 17, 247, 70, 149, 53, 205, 104, 161, 27, 141, 70, 85,
129, 249, 21, 154, 108, 5, 1, 11, 37, 135, 67, 255, 11, 14, 70, 68,
145, 128, 92, 202, 242, 74, 192, 98, 52, 77, 91, 245, 54, 149, 128, 108};

uint16_t random16()
{
    uint16_t ret = random8();
    ret |= (((uint16_t)random8())<<8);
    return ret;
}
