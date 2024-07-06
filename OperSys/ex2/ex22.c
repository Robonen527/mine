#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#define SIZE 150

void alarm_handler (int s) {}

int get_line(int file, char* buf) {
    int i = 0, ans;
    char c = 'a';
    do {
        ans = read(file, &c, 1);
        buf[i] = c;
        i++;
    } while((ans > 0) && (c != '\n'));
    buf[i - 1] = '\0';
    if (ans < 0) {
        writeErrorIn("read");
        return -1;
    }
    return 0;
}

// This func does all of the logic with directory of student.
int checkUser(char* dir, char* path, char* output, char* input) {
    DIR* dp;
    int isFile = 0, toRet = 100, err, output_curf, inputf, outputf;
    char dirPath[SIZE + 1] = {0};
    // getting the whole path of the directory.
    strcat(dirPath, path);
    strcat(dirPath, "/");
    strcat(dirPath, dir);
    if (!(dp = opendir(dirPath))) {
        writeErrorIn("opendir");
        return -1;
    }
    struct dirent* diren;
    char *filename;
    int i = 0, last = 0;
    while((diren = readdir(dp))) {
        if (!(strcmp(diren->d_name,".") && strcmp(diren->d_name, "..")))
            continue;
        filename = diren->d_name;
        // for getting the index of the last '.' for checking if it's a c file.
        i = 0; last = 0;
        while (filename[i] != '\0') {
            if (filename[i] == '.') {
                last = i;
            }
            i++;
        }
        // if its a c-file then eop = ".c\0" like we need.
        char eop[3] = {'.', filename[last + 1], filename[last + 2]};
        if (last != 0 && strcmp(eop, ".c") == 0) {
            isFile = 1;
            break;
        }
    }
    if (closedir(dp) < 0) {
        writeErrorIn("closedir");
        return -1;
    }
    if (!isFile)
        return 0;

    // getting the whole path of the file.
    strcat(dirPath, "/");
    strcat(dirPath, filename);
    char out[SIZE + 1] = {0};
    strcat(out, dirPath);
    strcat(out, ".out");
    char *args1[] = {"gcc", dirPath, "-o", out, NULL};
    pid_t pid;
    pid = fork();
    //gcc
    if (pid == -1) {
        writeErrorIn("fork");
        return -1;
    } else if (pid == 0) {
        if ((err = open("errors.txt", O_CREAT|O_APPEND|O_WRONLY, 777)) < 0) { 
		    writeErrorIn("open"); /* open failed */ 
		    exit(-1); 
	    } 
        if (dup2(err, STDERR_FILENO) == -1) {
            if (close(err) < 0) {
                writeErrorIn("close");
                exit(2);
            }
            writeErrorIn("dup2");
            exit(-1);
        }
        if (close(err) < 0) {
            writeErrorIn("close");
            exit(-1);
        }
        execvp("gcc", args1);
        writeErrorIn("execvp");
        exit(-1);
    } else {
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            writeErrorIn("waitpid");
            return -1;
        }
        if ( WIFEXITED(status) )
        {
            int exit_status = WEXITSTATUS(status);
            if (exit_status == -1)
                return -1;   
            if (exit_status)
                return 10;
        }
    }
    
    //a.out
    char *args2[] = {out, NULL};
    pid = fork();
    if (pid == -1) {
        writeErrorIn("fork");
        return -1;
    } else if (pid == 0) {
        if ((output_curf = open("output_cur.txt", O_CREAT|O_TRUNC|O_WRONLY, 0777)) < 0) { 
		    writeErrorIn("open"); /* open failed */ 
		    exit(-1); 
	    }
        if ((inputf = open(input, O_RDONLY)) < 0) { 
		    if (close(outputf) < 0) {
                writeErrorIn("close");
                exit(2);
            }
            writeErrorIn("open"); /* open failed */ 
		    exit(1); 
	    }
        if (dup2(inputf, STDIN_FILENO) == -1) {
            if (close(output_curf) < 0) {
                if (close(inputf) < 0) {
                    writeErrorIn("close");
                    exit(-1);
                }
                writeErrorIn("close");
                exit(-1);
            }
            if (close(inputf) < 0) {
                writeErrorIn("close");
                exit(-1);
            }
            writeErrorIn("dup2");
            exit(-1);
        }
        if (dup2(output_curf, STDOUT_FILENO) == -1) {
            if (close(output_curf) < 0) {
                if (close(inputf) < 0) {
                    writeErrorIn("close");
                    exit(2);
                }
                writeErrorIn("close");
                exit(2);
            }
            if (close(inputf) < 0) {
                writeErrorIn("close");
                exit(2);
            }
            writeErrorIn("dup2");
            exit(2);
        }
        if (close(output_curf) < 0) {
            if (close(inputf) < 0) {
                writeErrorIn("close");
                exit(2);
            }
            writeErrorIn("close");
            exit(2);
        }
        if (close(inputf) < 0) {
            writeErrorIn("close");
            exit(-1);
        }
        if (signal(SIGALRM, alarm_handler) < 0) {
            writeErrorIn("signal");
            exit(-1);
        }
        if (alarm(5) < 0) {
            writeErrorIn("alarm");
            exit(-1);
        }
        execvp(out, args2);
        writeErrorIn("execvp");
        exit(0);
    } else {////////////////////////////////////////////////////////////////////////////////
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            writeErrorIn("waitpid");
            if (unlink(out) < 0) {
                writeErrorIn("unlink");
                return -1;
            }
            return -1;
        }
        if (alarm(0) < 0) {
            writeErrorIn("alarm");
            return -1;
        }
        if (WIFSIGNALED(status)) {
            if (unlink(out) < 0) {
                writeErrorIn("unlink");
                return -1;
            }
            return 20;
        }
        if ( WIFEXITED(status) )
        {   
            int exit_status = WEXITSTATUS(status);
            if (exit_status) {
                if (unlink(out) < 0) {
                    writeErrorIn("unlink");
                    return -1;
                }
                return -1;
            }
        }
    }
    if (unlink(out) < 0) {
        writeErrorIn("unlink");
        return -1;
    }
    //run compare:
    char *args3[] = {"./comp.out", "output_cur.txt", output, NULL};
    pid = fork();
    if (pid == -1) {
        writeErrorIn("fork");
        return -1;
    } else if (pid == 0) {
        execvp("./comp.out", args3);
        writeErrorIn("execvp");
        exit(-1);
    } else {
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            writeErrorIn("waitpid");
            return -1;
        }
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);       
            switch (exit_status)
            {
            case 1:
                return 100;

            case 2:
                return 50;

            case 3:
                return 75;
            
            default:
                return -1;
                break;
            }
        }
    }

    return 100;
}

void writeErrorIn(char* err) {
    char ErrorIN[20] = "Error in: ";
    strcat(ErrorIN, err);
    strcat(ErrorIN, "\n");
    write(1, ErrorIN, strlen(ErrorIN));
}

int main(int argc, char* argv[]) {
    int result, err, output_curf;
    
    // openning errors.txt for add O_TRUNC.
    if ((err = open("errors.txt", O_CREAT|O_TRUNC|O_WRONLY, 0777)) < 0) { 
        writeErrorIn("open"); /* open failed */ 
        return -1; 
    }
    if (close(err) < 0) {
        writeErrorIn("close");
        return -1;
    }
    
    // openning the results file.
    if ((result = open("results.csv", O_CREAT|O_TRUNC|O_WRONLY, 0777)) < 0) { 
		    writeErrorIn("open"); /* open failed */ 
		    return -1; 
	}

    char dir[SIZE + 1] = {0}, output[SIZE + 1] = {0}, input[SIZE + 1] = {0};
    char *message, toWrite[SIZE+1] = {0};
    DIR* dp;
    struct dirent* diren;
    struct stat dir_stat;
    
    if (argc < 2) {
        message = "there isn't enough argumments\n";
        write(1, message, strlen(message));
        return -1;
    }

    int conf, ans;
    if ((conf = open(argv[1], O_RDONLY)) < 0) {
        writeErrorIn("open");
        return -1;
    }
    // lets get all of the lines.
    if (get_line(conf, dir) < 0)
        return -1;
    if (stat(dir, &dir_stat) || !S_ISDIR(dir_stat.st_mode)) {
        message = "Not a valid directory\n";
        write(1, message, strlen(message));
        return -1;
    }
    if (get_line(conf, input) < 0)
        return -1;
    if (stat(input, &dir_stat) || !S_ISREG(dir_stat.st_mode)) {
        message = "Input file not exist\n";
        write(1, message, strlen(message));
        return -1;
    }
    if (get_line(conf, output) < 0)
        return -1;
    if (stat(output, &dir_stat) || !S_ISREG(dir_stat.st_mode)) {
        message = "Output file not exist\n";
        write(1, message, strlen(message));
        return -1;
    }
    //if dp is null
    if(!(dp = opendir(dir))) {
        writeErrorIn("opendir");
        return -1;
    }
    //now for the main running
        while((diren = readdir(dp))) {
        if (!(strcmp(diren->d_name,".") && strcmp(diren->d_name, "..")))
            continue;
        if (diren->d_type == DT_DIR) {
            toWrite[0] = '\0';
            ans = checkUser(diren->d_name, dir, output, input);
            strcat(toWrite, diren->d_name);
            switch (ans)
            {
            case 0:
                strcat(toWrite, ",0,NO_C_FILE\n");
                break;
            
            case 10:
                strcat(toWrite, ",10,COMPILATION_ERROR\n");
                break;
            case 20:
                strcat(toWrite, ",20,TIMEOUT\n");
                break;
            case 50:
                strcat(toWrite, ",50,WRONG\n");
                break;
            case 75:
                strcat(toWrite, ",75,SIMILAR\n");
                break;
            case 100:
                strcat(toWrite, ",100,EXCELLENT\n");
                break;
            default:
                continue;
            }
            if (write(result, toWrite, strlen(toWrite)) < 0);
        }
    }
    if (unlink("output_cur.txt") < 0) {
        writeErrorIn("unlink");
        return -1;
    }
    if (closedir(dp) < 0) {
        writeErrorIn("closedir");
        return -1;
    }
    if (close(result) < 0) {
        writeErrorIn("close");
        return -1;
    }
}