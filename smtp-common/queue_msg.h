//
// Created by Никита Иксарица on 26.02.2022.
//

#ifndef SMTP_COMMON_QUEUE_MSG_H
#define SMTP_COMMON_QUEUE_MSG_H

#define QUEUE_MESSAGE_MAX_LENGTH 1024

typedef struct queue_msg
{
    // Тип сообщения. Должен принимать только положительные значения, используемые процессом-получателем для выбора
    // сообщений через функцию msgrcv.
    long mtype;
    // Содержание сообщения.
    char mtext[QUEUE_MESSAGE_MAX_LENGTH];
} queue_msg_t;

#endif //SMTP_COMMON_QUEUE_MSG_H
