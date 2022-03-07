//
// Created by Karpukhin Aleksandr on 06.03.2022.
//

#ifndef BMSTU_SMTP_PARSE_TEST_H
#define BMSTU_SMTP_PARSE_TEST_H

// Unit tests for all client commands parsing.
void before_test(void);
void after_test(void);

void test_parse_helo(void);
void test_parse_ehlo(void);
void test_parse_mail(void);
void test_parse_rcpt(void);
void test_parse_data(void);
void test_parse_data_int(void);
void test_parse_noop(void);
void test_parse_rset(void);
void test_parse_vrfy(void);
void test_parse_quit(void);

#endif //BMSTU_SMTP_PARSE_TEST_H
