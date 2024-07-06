#include <stdio.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define SIZE 10

char readChar(int fdin) {
    char c;
    int ans;
    do {
        ans = read(fdin, &c, 1);
    } while((c < 41) && (ans > 0));
    if ((c >= 'A') && (c <= 'Z'))
        c += 'a' - 'A';
    if (ans == 0)
        c = '\n';
    if (ans < 0)
        c = 0;
    return c;
}

int isTheSame(char* p1, char* p2) {
    int f1, f2, charr1, charr2;
    char buf1[SIZE + 1] = {0}, buf2[SIZE + 1] = {0}, *message;
    if ((f1 = open(p1, O_RDONLY)) < 0) {
        message = "Error in: open\n";
        write(1, message, strlen(message));
        return -1;
    }
    
    if ((f2 = open(p2, O_RDONLY)) < 0) {
        message = "Error in: open\n";
        write(1, message, strlen(message));
        return -1;
    }
    
    do {
        charr1 = read(f1, buf1, SIZE);
        if (charr1 < 0) {
            message = "Error in: read\n";
            write(1, message, strlen(message));
            return -1;
        }

        charr2 = read(f2, buf2, SIZE);
        if (charr2 < 0) {
            message = "Error in: read\n";
            write(1, message, strlen(message));
            return -1;
        }
        if (strcmp(buf1, buf2)) {
            close(f1);
            close(f2);
            return 0;       
        }
    } while ((charr1 == SIZE) && (charr2 == SIZE));
    close(f1);
    close(f2);
    return 1;
}

int main(int argc , char* argv[]) 
{
    char* message, c1, c2;
    int charr;
    if (argc < 3) {
        message = "there isn't enough argumments\n";
        write(1, message, strlen(message));
        return -1;
    }
    int same = isTheSame(argv[1], argv[2]);
    if (same == 1)
        return 1;
    if (same == -1)
        return -1;
    int f1, f2;
    if ((f1 = open(argv[1], O_RDONLY)) < 0) {
        message = "Error in: open\n";
        write(1, message, strlen(message));
        return -1;
    }
    
    if ((f2 = open(argv[2], O_RDONLY)) < 0) {
        message = "Error in: open\n";
        write(1, message, strlen(message));
        return -1;
    }

    //cehcks if they are similar. 
    while ((c1 != '\n') || (c2 != '\n')) {
        c1 = readChar(f1);
        if (c1 == 0) {
            message = "Error in: read\n";
            write(1, message, strlen(message));
            return -1;
        }
        c2 = readChar(f2);
        if (c2 == 0) {
            message = "Error in: read\n";
            write(1, message, strlen(message));
            return -1;
        }
        if (c1 != c2) {
            close(f1);
            close(f2);
            return 2;
        }
    }
    close(f1);
    close(f2);
    return 3;
}
