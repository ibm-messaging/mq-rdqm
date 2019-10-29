/*
 * (C) Copyright IBM Corporation 2019
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cmqc.h"

long int batchSize = 10;
long int numberOfBatches = 20;
long int sleepSeconds = 1;
long int verbosity = 0;

MQCHAR48 QMgrName;
size_t QMgrNameLen;
MQCHAR48 QName;
size_t QNameLen;
char UserId[13] = {0};
size_t UserIdLen = 0;
char Password[MQ_CSP_PASSWORD_LENGTH + 1] = {0};
size_t PasswordLen = 0;

int longValueOfOption(char *OptName, char *optarg, long int *valuep) {
    int rc = 0;
    long int value = 0;
    char *endptr;
    
    errno = 0;
    value = strtol(optarg, &endptr, 10);
    if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN)) ||
        (errno != 0 && value == 0)) {
        fprintf(stderr, "Invalid value \"%s\" for %s\n", optarg, OptName);
        rc = 1;
    } else if (endptr == optarg) {
        fprintf(stderr, "No digits were found for %s\n", OptName);
        rc = 1;
    } else if (*endptr != '\0') {
        fprintf(stderr,
                "Extra characters after number for %s: \"%s\"\n",
                OptName,
                endptr);
        rc = 1;
    }
    
    if (rc == 0) {
        *valuep = value;
    }
    
    return rc;
}

int processOptions(int argc, char *argv[]) {
    int rc = 0;
    int opt;
    char *password;
    unsigned long password_length;
    
    opterr = 0;
    
    while (((opt = getopt(argc, argv, "b:m:p:s:u:v:")) != -1) && (rc == 0)) {
        switch (opt) {
            case 'b':
                rc = longValueOfOption("numberOfBatches",
                                   optarg,
                                   &numberOfBatches);
                break;
            case 'm':
                rc = longValueOfOption("batchSize", optarg, &batchSize);
                break;
            case 'p':
                PasswordLen = strlen(optarg);
                if (PasswordLen < (MQ_CSP_PASSWORD_LENGTH + 1)) {
                    strncpy(Password, optarg, PasswordLen);
                } else {
                    fprintf(stderr, "Password too long\n");
                    rc = 1;
                }
                break;
            case 's':
                rc = longValueOfOption("sleepSeconds", optarg, &sleepSeconds);
                break;
            case 'u':
                UserIdLen = strlen(optarg);
                if (UserIdLen < 13) {
                    strncpy(UserId, optarg, UserIdLen);
                } else {
                    fprintf(stderr, "UserId too long\n");
                    rc = 1;
                }
                break;
            case 'v':
                rc = longValueOfOption("verbosity", optarg, &verbosity);
                break;
            default:
                fprintf(stderr, "Usage: rdqmget [-b batches] [-m messages] [-p password] [-s sleep] [-u userid] [-v verbosity] QMgrName QName\n");
                rc = 1;
        }
    }
    
    if ((UserIdLen > 0) && !(PasswordLen > 0)) {
        password = getpass("password:");
        if (password != NULL) {
            password_length = strlen(password);
            if (password_length < (MQ_CSP_PASSWORD_LENGTH + 1)) {
                strncpy(Password, password, password_length);
                PasswordLen = password_length;
            }
        }
    }
    
    if (verbosity > 2) {
        printf("numberOfBatches is %ld\n", numberOfBatches);
        printf("batchSize is %ld\n", batchSize);
        printf("sleepSeconds is %ld\n", sleepSeconds);
        printf("verbosity is %ld\n", verbosity);

    }
    
    if (rc == 0) {
        if ((argc - optind) == 2) {
            QMgrNameLen = strlen(argv[optind]);
            if (QMgrNameLen <= 48) {
                strncpy(QMgrName, argv[optind], QMgrNameLen);
                if (QMgrNameLen < 48) {
                    QMgrName[QMgrNameLen] = '\0';
                }
                if (verbosity > 2) {
                    printf("QMgrName is \"%48s\"\n", QMgrName);
                }
                QNameLen = strlen(argv[optind + 1]);
                if (QNameLen <= 48) {
                    strncpy(QName, argv[optind + 1], QNameLen);
                    if (QNameLen < 48) {
                        QName[QNameLen] = '\0';
                    }
                    if (verbosity > 2) {
                        printf("QName is \"%48s\"\n", QName);
                    }
                } else {
                    fprintf(stderr, "Queue Name too long\n");
                    rc = 1;
                }
            } else {
                fprintf(stderr, "Queue Manager Name too long\n");
                rc = 1;
            }
        } else {
            fprintf(stderr, "Usage: rdqmget [-b batches] [-m messages] [-s sleep] [-v verbosity] QMgrName QName\n");
            rc = 1;
        }
    }
    
    return rc;
}
