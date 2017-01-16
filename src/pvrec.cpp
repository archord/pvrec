//============================================================================
// Name        : pvrec.cpp
// Author      : Xiaomeng Lu
// Version     :
// Copyright   : Your copyright notice
// Description : 使用霍夫变换提取位置变化源
//============================================================================

#include <iostream>
#include "APVRec.h"
#include <sys/time.h>
#include <errno.h>

using namespace std;
using namespace AstroUtil;

/*
void resolve_line(const char* line, pv_point& pt) {
        int hh, mm, skyid;
        double ss;

        // 格式要求(要求)
        sscanf(line, "%d %d/%d/%d %d:%d:%d %lf %lf %lf %lf %lf",
                        &pt.fno, &pt.year, &pt.month, &pt.day, &hh, &mm, &ss,
                        &pt.ra, &pt.dec, &pt.x, &pt.y, &pt.mag);
        pt.secs = hh * 3600 + mm * 60 + ss;
}

int main(int argc, char** argv) {
        FILE* fp = fopen(argv[1], "r");
        char lnbuff[100];
        pv_point pt1, pt2, pt;
        hough_point hpt;
        double r, dr;
        int npt(0);
        int n;

        while(!feof(fp)) {// 遍历文件, 解析行数据
                if (fgets(lnbuff, 100, fp) == NULL) continue;
                resolve_line(lnbuff, pt);
                if (++npt == 1)
                        pt1 = pt;
                else if (npt == 2) {
                        pt2 = pt;
                        n = pt2.fno - pt1.fno;
                        hpt.line2hough(pt1.x, pt1.y, pt2.x, pt2.y);
                        printf("(%6.1f, %6.1f; %6.1f, %6.1f) ==> (%6.1f, %6.1f), (%7.1f, %5.1f, %7.1f, %7.1f)\n",
                                        pt1.x, pt1.y, pt2.x, pt2.y,
                                        (pt2.x - pt1.x) / n, (pt2.y - pt1.y) / n,
                                        hpt.r, hpt.theta * RtoG, hpt.x, hpt.y);
                }
                else {
                        r = pt.x * cos(hpt.theta) + pt.y * sin(hpt.theta);
                        dr = r - hpt.r;
                        pt1 = pt2;
                        pt2 = pt;
                        n = pt2.fno - pt1.fno;
                        hpt.line2hough(pt1.x, pt1.y, pt2.x, pt2.y);
                        printf("(%6.1f, %6.1f; %6.1f, %6.1f) ==> (%6.1f, %6.1f), (%7.1f, %5.1f, %7.1f, %7.1f), (%7.1f, %7.1f)\n",
                                        pt1.x, pt1.y, pt2.x, pt2.y,
                                        (pt2.x - pt1.x) / n, (pt2.y - pt1.y) / n,
                                        hpt.r, hpt.theta * RtoG, hpt.x, hpt.y,
                                        r, dr);
                }
        }
        fclose(fp);

        return 0;
}
 */
///*

void resolve_line(const char* line, pv_point& pt) {
  int hh, mm, skyid;
  double ss;

  // 格式要求(要求)
  // X  Y  RA  DEC  YYYY/MM/DD HH:MM:SS.SSS  Mag  FrameNo  SkyID
  sscanf(line, "%lf %lf %lf %lf %d/%d/%d %d:%d:%lf %lf %d %d",
          &pt.x, &pt.y, &pt.ra, &pt.dec,
          &pt.year, &pt.month, &pt.day, &hh, &mm, &ss,
          &pt.mag,
          &pt.fno, &skyid);
  pt.secs = hh * 3600 + mm * 60 + ss;
  pt.secs += 5; // 加曝光时间/2修正量, 14 Oct
}

//void SS2HMS(double seconds, int& hh, int& mm, int& ss, int& microsec) {

void SS2HMS(double seconds, int& hh, int& mm, double& ss) {
  hh = (int) (seconds / 3600);
  seconds -= (hh * 3600);
  mm = (int) (seconds / 60);
  ss = seconds - mm * 60;
  //	seconds -= (mm * 60);
  //	ss = (int) seconds;
  //	microsec = (int) ((seconds - ss) * 1E6);
}

int main(int argc, char** argv) {
  //	if (argc != 3) {
  //		cout << "Usgae: pvrec <path name of raw file> <directory name of result>" << endl;
  //		return -1;
  //	}
  char *pathraw = "/home/xy/gwac-data/170108/170108-8-30.txt";
  char *dirrslt = "/home/xy/gwac-data/170108";
  char pathrslt[200];
  int n, i(0);
  FILE *fpraw;
  FILE *fprslt;
  char lnbuff[100];
  pv_point pt;
  APVRec pvrec;

  //	strcpy(pathraw, argv[1]);
  //	strcpy(dirrslt, argv[2]);
  //	n = strlen(dirrslt);
  //	if (dirrslt[n - 1] != '/') {
  //		dirrslt[n] = '/';
  //		dirrslt[n + 1] = 0;
  //	}

  if (NULL == (fpraw = fopen(pathraw, "r"))) {
    cout << "Failed to open RAW file: " << strerror(errno) << endl;
    return -2;
  }

  int frmno(-1), nfrm(0);
  timeval tv;
  double t1, t2;

  gettimeofday(&tv, NULL);
  t1 = tv.tv_sec + tv.tv_usec * 1E-6;
  n = 0;
  pvrec.new_sequence();
  while (!feof(fpraw)) {// 遍历文件, 解析行数据
    if (fgets(lnbuff, 100, fpraw) == NULL) continue;
    resolve_line(lnbuff, pt);
    ++n;
    if (pt.fno != frmno) {
      pvrec.end_frame();
      pvrec.new_frame(frmno, pt.secs);
      frmno = pt.fno;
      ++nfrm;
    }
    pvrec.add_point(&pt);
  }
  pvrec.end_frame();
  pvrec.end_sequence();
  gettimeofday(&tv, NULL);
  t2 = tv.tv_sec + tv.tv_usec * 1E-6;
  printf("%4d frames have %5d total points. Ellapsed %.3f seconds\n", nfrm, n, t2 - t1);

  fclose(fpraw);

  PPVOBJLNK objs = pvrec.get_object();
  PPVPTLNK ptptr;
  PPVPT ppt;
  int hh, mm;
  double ss;

  n = 0;
  while ((objs = objs->next) != NULL) {
    ptptr = objs->object.pthead;
    sprintf(pathrslt, "%s/%04d.txt", dirrslt, ++i);
    if ((fprslt = fopen(pathrslt, "w")) == NULL) {
      cout << pathrslt << " : open failed < " << strerror(errno) << " >" << endl;
      continue;
    }
    //		printf("object %4d : consists of %3d points\n", i, objs->object.npt);
    //		fprintf(fprslt, "%4s %3s %4s %2s %2s %2s %2s %2s %6s %7s %7s %9s %9s %7s\n",
    //				"f_no", "f_i", "YY", "MM", "DD", "hh", "mm", "ss", "mics", "X", "Y", "R.A.", "DEC.", "Mag");
    while ((ptptr = ptptr->next) != NULL) {
      ++n;
      ppt = ptptr->pt;
      SS2HMS(ppt->secs, hh, mm, ss);
      fprintf(fprslt, "%4d %3d %4d %2d %2d %2d %2d %6.3f %7.2f %7.2f %9.5f %9.5f %7.3f\n",
              ppt->fno, ppt->fi,
              ppt->year, ppt->month, ppt->day, hh, mm, ss,
              ppt->x, ppt->y, ppt->ra, ppt->dec, ppt->mag);
    }

    fclose(fprslt);
  }
  printf("%4d points are totally correlated. %4d segments are recognized\n", n, i);
  //	cout << n << " points are totally correlated" << endl;
  cout << "---------- Over ----------" << endl;

  return 0;
}
//*/
