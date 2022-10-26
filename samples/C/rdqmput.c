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
extern MQHOBJ Hobj;
MQCSP csp = {MQCSP_DEFAULT};

/*
 * Put a single message using MQPUT.
 *
 * If we need to retry to put a single message, we do that here.
 *
 * If we need to call MQBACK and retry the entire batch, we do that in
 * putBatch.
 *
 * If we get another error we just log it.
 *
 * Return values:
 *     0 - success
 *     1 - need to call MQBACK and retry
 *     2 - some other error
 */
int putMessage(int batchNumber, int messageNumber) {
    int rc = 0;
    MQPMO PutMsgOpts = {MQPMO_DEFAULT};
    MQMD MsgDesc = {MQMD_DEFAULT};

    PutMsgOpts.Options = MQPMO_SYNCPOINT     |
                         MQPMO_NEW_MSG_ID    |
                         MQPMO_NEW_CORREL_ID |
                         MQPMO_FAIL_IF_QUIESCING;

    snprintf(messageBuffer,
             messageSize - 1,
             "Batch %d, Message %d",
             batchNumber,
             messageNumber);
    ((char *)(messageBuffer))[messageSize - 1] = '\0';

    MsgDesc.Persistence = MQPER_PERSISTENT;

    if (verbosity > 1) {
        printf("About to put message %d of batch %d\n",
               messageNumber,
               batchNumber);
    }

    if (verbosity > 2) {
        printf("Message is \"%s\"\n", (char *)messageBuffer);
    }

    MQPUT(Hconn,
          Hobj,
          &MsgDesc,
          &PutMsgOpts,
          messageSize,
          messageBuffer,
          &CompCode,
          &Reason);
    if (CompCode == MQCC_OK) {
        if (verbosity > 1) {
            printf("Message %d put successfully\n", messageNumber);
        }
    } else if (CompCode == MQCC_FAILED) {
        if (Reason == MQRC_BACKED_OUT) {
            rc = 1;
            fprintf(stderr,
                    "MQPUT failed with MQRC_BACKED_OUT\n");
        } else if (Reason == MQRC_CALL_INTERRUPTED) {
            /*
             * I have never seen this reason here but handle it just in case.
             *
             * In this case, the safest thing is to backout the
             * batch and try again, just like with MQRC_BACKED_OUT
             */
            fprintf(stderr,
                    "MQPUT failed with MQRC_CALL_INTERRUPTED\n");
            rc = 1;
        } else {
            // Just give up and log the error
            rc = 2;
            logFailure("MQPUT", CompCode, Reason);
        }
    } else {
        // Just give up and log the error
        rc = 2;
        logFailure("MQPUT", CompCode, Reason);
    }

    return rc;
}

/*
 * Put one batch of messages.
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
int putBatch(int batchNumber) {
    int messageNumber = 1;
    int putBatchRc = 0;
    int putMessageRc = 0;
    int backoutRc = 0;
    int commitRc = 0;

    while (messageNumber <= batchSize) {
        putMessageRc = putMessage(batchNumber, messageNumber);
        if (putMessageRc == 0) {
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
                if (putMessageRc == 1) {
                    // Retry the batch
                    messageNumber = 1;
                } else {
                    // Give up
                    putBatchRc = 1;
                    break;
                }
            } else {
                putBatchRc = 1;
                break;
            }
        }
        if (messageNumber == (batchSize + 1)) {
            // Have reached the end of the batch
            if (putBatchRc == 0) {
                if (verbosity > 0) {
                    printf("Batch %d put successfully, committing...\n",
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
                    putBatchRc = 1;
                }
            } else {
                fprintf(stderr,
                        "Batch %d failed, backed out...\n",
                        batchNumber);
            }
        }
    }

    return putBatchRc;
}

int main(int argc, char *argv[]) {
    int exitCode = 0;
    MQCHAR48 QMgrName;
    MQCNO ConnectOpts = {MQCNO_DEFAULT};
    MQLONG Options;
    MQOD ObjDesc = {MQOD_DEFAULT};
    int currentBatch = 1;
    int putBatchRc = 0;
    int optionsRc = 0;
    int openRc = 0;
    
    optionsRc = processOptions(argc, argv);
    if (optionsRc == 0) {
        openRc = connect_and_open(MQOO_OUTPUT | MQOO_FAIL_IF_QUIESCING);
        if (openRc == 0) {
            while ((putBatchRc == 0) &&
                   (currentBatch <= numberOfBatches)) {
                putBatchRc = putBatch(currentBatch);
                currentBatch++;
            }
            printf("Completed\n");
            if (putBatchRc != 0) {
                exitCode = 4;
            }
            close_and_disconnect();
        }
    } else {
        exitCode = 1;
    }

    exit(exitCode);
}
