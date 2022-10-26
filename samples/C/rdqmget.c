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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cmqc.h"
#include "complete.h"
#include "connection.h"
#include "globals.h"
#include "log.h"
#include "options.h"

extern void *messageBuffer;

MQLONG CompCode;
MQLONG Reason;
MQCSP csp = {MQCSP_DEFAULT};

/*
 * Get a single message using MQGET.
 *
 * If we need to retry to get a single message, we do that here.
 *
 * If we need to call MQBACK and retry the entire batch, we do that in
 * getBatch.
 *
 * If we get another error we just log it.
 *
 * Return values:
 *     0 - success
 *     1 - need to call MQBACK and retry
 *     2 - some other error
 */
int getMessage(int batchNumber, int messageNumber) {
    int rc = 0;
    MQGMO GetMsgOpts = {MQGMO_DEFAULT};
    MQMD MsgDesc = {MQMD_DEFAULT};
    MQLONG messlen;

    GetMsgOpts.Version = MQGMO_VERSION_2;
    GetMsgOpts.MatchOptions = MQMO_NONE;
    GetMsgOpts.Options = MQGMO_SYNCPOINT            |
                         MQGMO_WAIT                 |
                         MQGMO_ACCEPT_TRUNCATED_MSG |
                         MQGMO_CONVERT;
    GetMsgOpts.WaitInterval = MQWI_UNLIMITED;

    if (verbosity > 1) {
        printf("About to get message %d of batch %d\n",
               messageNumber,
               batchNumber);
    }

    MQGET(Hconn,
          Hobj,
          &MsgDesc,
          &GetMsgOpts,
          messageSize,
          messageBuffer,
          &messlen,
          &CompCode,
          &Reason);
    if (CompCode == MQCC_OK) {
        if (verbosity > 1) {
            printf("Message %d got successfully\n", messageNumber);
        }
        if (verbosity > 2) {
            printf("Message is \"%s\"\n", (char *)messageBuffer);
        }
    } else if (CompCode == MQCC_WARNING) {
        if (Reason == MQRC_TRUNCATED_MSG_ACCEPTED) {
            if (verbosity > 2) {
                // Terminate buffer as original message was longer
                ((char *)(messageBuffer))[messageSize - 1] = '\0';
                printf("Truncated Message is \"%s\"\n", (char *)messageBuffer);
            }
        } else {
            // Just give up and log the error
            rc = 2;
            logFailure("MQGET", CompCode, Reason);
        }
    } else if (CompCode == MQCC_FAILED) {
        if (Reason == MQRC_BACKED_OUT) {
            rc = 1;
            fprintf(stderr,
                    "MQGET failed with MQRC_BACKED_OUT\n");
        } else if (Reason == MQRC_CALL_INTERRUPTED) {
            /*
             * I have never seen this reason here but handle it just in case.
             *
             * In this case, the safest thing is to backout the
             * batch and try again, just like with MQRC_BACKED_OUT
             */
            fprintf(stderr,
                    "MQGET failed with MQRC_CALL_INTERRUPTED\n");
            rc = 1;
        } else {
            // Just give up and log the error
            rc = 2;
            logFailure("MQGET", CompCode, Reason);
        }
    } else {
        // Just give up and log the error
        rc = 2;
        logFailure("MQGET", CompCode, Reason);
    }

    return rc;
}

/*
 * Get one batch of messages.
 *
 * If we need to call MQBACK and retry the current batch then
 * that is done here.
 *
 * If we get any other error for one message then call MQBACK
 * and give up.
 *
 * Return values:
 *     0 - success
 *     1 - failure
 */
int getBatch(int batchNumber) {
    int messageNumber = 1;
    int getBatchRc = 0;
    int getMessageRc = 0;
    int backoutRc = 0;
    int commitRc = 0;

    while (messageNumber <= batchSize) {
        getMessageRc = getMessage(batchNumber, messageNumber);
        if (getMessageRc == 0) {
            messageNumber++;
            if (sleepSeconds > 0) {
                sleep(sleepSeconds);
            }
        } else {
            backoutRc = backout(Hconn);
            if (backoutRc == 0) {
                messageNumber = 1;
                if (verbosity > 0) {
                    printf("backout successful\n");
                }
                if (getMessageRc == 1) {
                    // Retry the batch
                    messageNumber = 1;
                } else {
                    // Give up
                    getBatchRc = 1;
                    break;
                }
            } else {
                getBatchRc = 1;
                break;
            }
        }
        if (messageNumber == (batchSize + 1)) {
            // Have reached the end of the batch
            if (getBatchRc == 0) {
                if (verbosity > 0) {
                    printf("Batch %d got successfully, committing...\n",
                           batchNumber);
                }
                commitRc = commit(Hconn);
                if (commitRc == 0) {
                    if (verbosity > 0) {
                        printf("Batch %d committed successfully\n",
                               batchNumber);
                    }
                } else if (commitRc == 1) {
                    // Need to retry current batch.
                    messageNumber = 1;
                } else {
                    getBatchRc = 1;
                }
            } else {
                fprintf(stderr,
                        "Batch %d failed, backed out...\n",
                        batchNumber);
            }
        }
    }

    return getBatchRc;
}

int main(int argc, char *argv[]) {
    int exitCode = 0;
    MQCNO ConnectOpts = {MQCNO_DEFAULT};
    MQLONG Options;
    MQOD ObjDesc = {MQOD_DEFAULT};
    int currentBatch = 1;
    int getBatchRc = 0;
    int optionsRc = 0;
    int openRc = 0;
    
    optionsRc = processOptions(argc, argv);
    if (optionsRc == 0) {
        openRc = connect_and_open(MQOO_INPUT_AS_Q_DEF | MQOO_FAIL_IF_QUIESCING);
        if (openRc == 0) {
            while ((getBatchRc == 0) &&
                   (currentBatch <= numberOfBatches)) {
                getBatchRc = getBatch(currentBatch);
                currentBatch++;
            }
            printf("Completed\n");
            if (getBatchRc != 0) {
                exitCode = 4;
            }
            close_and_disconnect();
        }
    } else {
        exitCode = 1;
    }

    exit(exitCode);
}
