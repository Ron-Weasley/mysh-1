#ifndef SIGNAL_H_
#define SIGNAL_H_

struct sigaction{
     void (*sa_handler)(int);
     int sa_mask;
     int sa_flags;
};

void catch_sigint(int signalNo);

void catch_sigtstp(int signalNo);

#endif // SIGNAL_H_
