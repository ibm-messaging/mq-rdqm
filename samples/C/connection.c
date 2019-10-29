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
#include <string.h>

#include "cmqc.h"
#include "connection.h"
#include "globals.h"
#include "log.h"
#include "options.h"

int connect_and_open(MQLONG Options) {
    int rc = 0;
    MQCNO ConnectOpts = {MQCNO_DEFAULT};
    MQCSP csp = {MQCSP_DEFAULT};
    MQLONG CompCode;
    MQLONG Reason;
    MQOD ObjDesc = {MQOD_DEFAULT};

    ConnectOpts.Options = MQCNO_RECONNECT;
    if (strlen(UserId) > 0) {
        csp.AuthenticationType = MQCSP_AUTH_USER_ID_AND_PWD;
        csp.CSPUserIdPtr = UserId;
        csp.CSPUserIdLength = UserIdLen;
        csp.CSPPasswordPtr = Password;
        csp.CSPPasswordLength = PasswordLen;
        ConnectOpts.SecurityParmsPtr = &csp;
        ConnectOpts.Version = MQCNO_VERSION_5;
    }

    MQCONNX(QMgrName, &ConnectOpts, &Hconn, &CompCode, &Reason);

    if (CompCode == MQCC_OK) {
        printf("Connected to queue manager %s\n", QMgrName);
        strncpy(ObjDesc.ObjectName, QName, QNameLen);
        MQOPEN(Hconn, &ObjDesc, Options, &Hobj, &CompCode, &Reason);
        if (CompCode == MQCC_OK) {
            printf("Opened queue %s\n", ObjDesc.ObjectName);
        } else {
            logFailure("MQOPEN", CompCode, Reason);
            rc = 1;
        }
    } else {
        logFailure("MQCONNX", CompCode, Reason);
        rc = 3;
    }

    return rc;
}

void close_and_disconnect() {
    MQLONG CompCode;
    MQLONG Reason;

    MQCLOSE(Hconn, &Hobj, MQCO_NONE, &CompCode, &Reason);
    if (CompCode != MQCC_OK) {
        logFailure("MQCLOSE", CompCode, Reason);
    }
    MQDISC(&Hconn, &CompCode, &Reason);
    if (CompCode != MQCC_OK) {
        logFailure("MQDISC", CompCode, Reason);
    }

    return;
}
