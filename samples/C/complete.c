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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmqc.h"
#include "complete.h"
#include "log.h"

int backout(MQHCONN Hconn) {
    int rc = 0;
    MQLONG CompCode;
    MQLONG Reason;

    MQBACK(Hconn, &CompCode, &Reason);
    if (CompCode != MQCC_OK) {
        rc = 1;
        logFailure("MQBACK", CompCode, Reason);
    }

    return rc;
}

/*
 * Commit the current syncpoint.
 *
 * Return values:
 *     0 - success
 *     1 - commit was backed out so current batch can be retried
 *     2 - some other error so give up
 */
int commit(MQHCONN Hconn) {
    int rc = 0;
    MQLONG CompCode;
    MQLONG Reason;

    MQCMIT(Hconn, &CompCode, &Reason);
    if (CompCode != MQCC_OK) {
        rc = 2;
        if (CompCode == MQCC_FAILED) {
            if (Reason == MQRC_BACKED_OUT) {
                fprintf(stderr,
                        "MQCMIT failed with MQCC_FAILED and MQRC_BACKED_OUT\n");
                rc = 1;
            } else if (Reason == MQRC_CALL_INTERRUPTED) {
                fprintf(stderr,
                        "MQCMIT failed with MQCC_FAILED and MQRC_CALL_INTERRUPTED\n");
                rc = 1;
            } else {
                logFailure("MQCMIT", CompCode, Reason);
            }
        } else {
            logFailure("MQCMIT", CompCode, Reason);
        }
    }

    return rc;
}
