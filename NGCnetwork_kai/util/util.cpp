#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "util.h"


// todo: ���Ɍ^�ϊ�����K�v�͂Ȃ�����

// ValList.cpp�Ŏg���Ă��� for debug?
void error(char* msg)
{
    fprintf(stderr, "%s\n", msg);
    abort();
}

void report_time()
{
    struct rusage   rusage;
    getrusage(RUSAGE_SELF, &rusage);
    long utime_s = rusage.ru_utime.tv_sec;
    long utime_u = rusage.ru_utime.tv_usec;
    double utime = (double) utime_s + ((double) utime_u / 1e6);
    long stime_s = rusage.ru_stime.tv_sec;
    long stime_u = rusage.ru_stime.tv_usec;
    double stime = (double) stime_s + ((double) stime_u / 1e6);
    fprintf(stderr, "Time %.2fu + %.2fs = %.2f sec.\n",
            utime, stime, utime+stime);
}

//////////////////////////////////////////////////////////////////

int time_expire;
int timer_value;

static struct itimerval value = {{0, 0}, {0, 0}};
//     value.it_interval.tv_sec = 0;
//     value.it_interval.tv_usec = 0;
//     value.it_value.tv_sec = 100;
//     value.it_value.tv_usec = 0;

void timeout(int sig)
{
    fprintf(stderr, "timeout by SIGPROF[%d], %d sec\n", sig, timer_value);
    time_expire = 1;
    return;
}

void setTimer(int sec)
{
    ::value.it_value.tv_sec = sec;
    ::value.it_value.tv_usec = 0;
    setitimer(ITIMER_PROF, &::value, 0);
    signal(SIGPROF, timeout);
    timer_value = sec;
    time_expire = 0;
}

int isTimerExpire()
{
    return time_expire;
}

//void my_abort(const char* c)
void my_abort(char* c)
{
    fprintf(stderr, "Error: %s\n", c);
    abort();
}

//////////////////////////////////////////////////////////////////

void ifile_error(int row, char *s) {
    fprintf(stderr, "In line %d, %s.\n", row, s);
    abort();
}

////	PLA, BLIF �p�C1�s�ǂ݃��[�`��
////	aline �� `\0` �ŏI��邱��
////	�s����EOF�̂Ƃ��̂�aline[0]�� '\0'������
void readline(FILE* filep, char*& aline, int& alineSize, int& row)
{
    int i=0;
    char* newaline;
    while( 1 ) {
        aline[i] = getc(filep);
        if ( aline[i] == EOF ) {
            aline[i] = '\0';
            break;
        }
        if ( aline[i] == '#' || aline[i] == '\n' ) {	// �R�����g or �s��
            // �R�����g��������s���܂œǂݔ�΂�
            if ( aline[i] == '#' ) while( getc(filep) != '\n' );
            row++;
            if ( i > 0 ) {			// ���Ȃ��Ƃ�1��������Ζ߂�
                aline[i] = '\0';
                break;
            }
            else continue;		
        }
        else if ( aline[i] == '\\' ) {		// ���̍s���Ȃ�
            aline[i] = getc(filep);
            if ( aline[i] == '\n' ) {
                aline[i] = getc(filep);
                row++;
            }
            else ifile_error(row,
                             (char*)", '\\' should be the last character of a line.\n");
        }
        if ( i++ >= alineSize-1 ) {	// ���̕�����
            newaline = new char[alineSize<<1];
            if ( newaline == 0 ) assert(!(int)"no more memory");
            memcpy(newaline, aline, sizeof(char)*alineSize);
            delete [] aline;
            aline = newaline;
            newaline = 0;
            alineSize <<= 1;
        }
    }
}

int word_cnt(const char* c) {
    int count=0;
    while ( *c != '\0' ) {
        while ( *c == ' ' ) c++;
        if ( *c != '\0' ) {
            count++;
            while ( *c != ' ' && *c != '\0' ) c++;
        }
    }
    return count;
}

// marimo:
// a bb cc�Ȃ�return a?
// stringstream�ł���
static const std::string sep = std::string(" ") + std::string("\0");

std::string getAword(const char* c)
{
    assert( *c != ' ' );	// 1�����ڂ͋󔒂łȂ�����

    std::string str = std::string(c);

    int start = str.find_first_not_of(sep); // ���݂̕�����̃C���f�b�N�X�Ԗڂ̕������猟�����J�n���A������̒��Ɋ܂܂�Ȃ��������ŏ��Ɍ��������ʒu��Ԃ��B������Ȃ��ꍇ�ɂ�string::npos��Ԃ�
    int stop = str.find_first_of(sep); // ���݂̕�����̃C���f�b�N�X�Ԗڂ̕������猟�����J�n���A������̒��Ɋ܂܂�镶�����ŏ��Ɍ��������ʒu��Ԃ��B������Ȃ��ꍇ�ɂ�string::npos��Ԃ�

    return str.substr(start, stop-start); // ���݂̕�����̃C���f�b�N�X ����n�܂蒷�� �̕��������񕔕��������Ԃ�
}

int hash_string(const std::string& key) // From Dragon book, p436
{
    const char* x = key.c_str();

    unsigned int h = 0;
    unsigned int g;

    while (*x != 0)
    {
        h = (h << 4) + *x++;
        if ((g = h & 0xf0000000) != 0)
            h = (h ^ (g >> 24)) ^ g;
    }
    return h;
}
