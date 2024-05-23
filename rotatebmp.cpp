#include <fstream>
#include <iostream>
using namespace std;
// 定义调试级别
#define DEBUG_LEVEL 0
// 根据调试级别决定是否打印调试信息
#if DEBUG_LEVEL > 0
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

// 定义单个像素
struct pixel {
  char B;
  char G;
  char R;
};
struct pixelwith {
  int x;
  int y;
};
// 定义单个像素坐标
int main(int argc, char *argv[]) {
  DEBUG_PRINT(sizeof(pixel));
  char flag[3];
  int filesize;
  // 创建fstream对象并打开要旋转的对象
  // 0L
  fstream rfile;
  rfile.open(argv[1], ios::in | ios::binary);
  DEBUG_PRINT("successfully open");
  // 标识符
  rfile.read((char *)(&flag), 2L);
  // 2L
  flag[2] = '\0';
  DEBUG_PRINT(flag);
  // 文件格式错误报错
  if (!(flag[0] == 'B' && flag[1] == 'M')) {
    cout << "format worng! need .bmp file" << endl;
    return 0;
  }
  rfile.read((char *)(&filesize), 4L);
  // 6L
  DEBUG_PRINT(filesize);
  // 保留字1,2
  char reservedWord[4];
  rfile.read((char *)(&filesize), 4L);
  // 10L
  // 图像偏移量,对应图像数据的起始位置
  int startlocation;
  rfile.read((char *)(&startlocation), 4L);
  // 14L
  DEBUG_PRINT(startlocation);
  long curptr = rfile.tellg();
  if (curptr != 14) {
    cout << "file head read error!" << endl;
    return 0;
  }
  // 文件头结束,共计14L
  DEBUG_PRINT("curptr" << ' ' << curptr << ' ' << 'L');

  int imageheadsize; // 图像头大小
  rfile.read((char *)(&imageheadsize), 4L);
  // 18L
  int width, height; // 图像的宽度和高度
  rfile.read((char *)(&width), 4L);
  rfile.read((char *)(&height), 4L);
  DEBUG_PRINT(width);
  DEBUG_PRINT(height);
  // 26L
  int colorspace, colordepth; // 颜色平面,颜色深度
  rfile.read((char *)(&colorspace), 2L);
  rfile.read((char *)(&colordepth), 2L);
  // 30L
  int zipway, imagesize; // 压缩方式,图片大小
  rfile.read((char *)(&zipway), 4L);
  rfile.read((char *)(&imagesize), 4L);
  DEBUG_PRINT(imagesize);
  // 38L
  int lrppi, udppi; // 水平分辨率,垂直分辨率
  rfile.read((char *)(&lrppi), 4L);
  rfile.read((char *)(&udppi), 4L);
  // 46L
  int tcolornumber; // 调色板颜色数
  rfile.read((char *)(&tcolornumber), 4L);
  // 50L
  int tcolor; // 调色板
  rfile.read((char *)(&tcolor), 4L);
  DEBUG_PRINT(tcolor);
  // 54L
  char head[54];
  rfile.seekg(ios::beg);
  rfile.read((char *)(&head), 54L);
  long curposition = rfile.tellg();
  DEBUG_PRINT(curposition);
  // 图像头结束,共计54L
  // 图像数据开始
  // 像素从左到右、从上到下排列。
  // 考虑读取对齐,对于每一行的数据,它必须是4的倍数,如果不是则需要对齐(补充1到3个字节)
  int rcomplement = (4 - (width * 3) % 4) % 4;  // 读取对齐参数
  int wcomplement = (4 - (height * 3) % 4) % 4; // 写入对齐参数
  DEBUG_PRINT("rcomplement" << ' ' << rcomplement);
  DEBUG_PRINT("wcomplement" << ' ' << wcomplement);
  // 逆时针
  // 矩阵形式 (x,y) (H,W) ==> (y,H-1-x) (W,H)
  // 行形式 (x*W+y) ==> (y*H+(H-1-x))
  // 其中N为宽度width,M为高度height
  // 顺时针
  // 矩阵形式 (x,y) (H,W) ==> (W-1-y,x) (W,H)
  // 行形式 (x*W+y) ==> ((W-1-y)*H+x)
  // 其中N为宽度width,M为高度height

  DEBUG_PRINT(sizeof(pixel));

  pixel *oldimage = new pixel[width * height];
  pixelwith *oldimageindex = new pixelwith[width * height];

  for (int x = 0; x < height; x++) {
    for (int y = 0; y < width; y++) {
      rfile.read((char *)(&oldimage[x * width + y]), 3);
      oldimageindex[x * width + y].x = x;
      oldimageindex[x * width + y].y = y;
    }
    char temp[4] = {0};
    rfile.read((char *)(&temp), rcomplement);
  }
  rfile.close();

  DEBUG_PRINT("read process end");

  // 成功读取了色彩信息 和 坐标信息

  pixel *newimage = new pixel[width * height];

  for (int x = 0; x < height; x++) {
    for (int y = 0; y < width; y++) {
      // 计算每一个点在新图像中的位置
      int newIndex = (width - y - 1) * height + x;
      newimage[newIndex] = oldimage[x * width + y];
    }
  }
  int *width_r = (int *)(&head[18]);
  (*width_r) = height;
  int *height_r = (int *)(&head[22]);
  (*height_r) = width;
  int *lrppi_r = (int *)(&head[38]);
  (*lrppi_r) = udppi;
  int *udppi_r = (int *)(&head[42]);
  (*udppi_r) = lrppi;
  DEBUG_PRINT("data process end");
  fstream wfile;
  wfile.open(argv[2], ios::out | ios::binary);
  wfile.write((char *)(&head), 54);
  DEBUG_PRINT("write process start");
  // 写入新图片
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      int newIndex = x * height + y;
      wfile.write((char *)(&newimage[newIndex]), 3);
    }
    char temp[4] = {0};
    wfile.write((char *)(&temp), wcomplement);
  }
  wfile.close();
  DEBUG_PRINT("write process end");
  delete[] oldimage;
  delete[] oldimageindex;
  delete[] newimage;
  DEBUG_PRINT("free space end");
  return 0;
}