//
// Created by Karpukhin Aleksandr on 06.03.2022.
//

#include <stdio.h>

#include "parse_test.h"

int main(void) {
    printf("\nParsing tests:\n");
    before_test();

    test_parse_helo();
    test_parse_ehlo();

    test_parse_mail();
    test_parse_rcpt();

    test_parse_data();
    test_parse_data_int();

    test_parse_noop();
    test_parse_rset();
    test_parse_vrfy();
    test_parse_quit();

    after_test();

    return 0;
}
