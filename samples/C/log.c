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

#include "cmqc.h"

void logFailure(char *call, MQLONG CompCode, MQLONG Reason) {
    if (CompCode == MQCC_WARNING) {
        fprintf(stderr, "%s returned MQCC_WARNING\n", call);
    } else if (CompCode == MQCC_FAILED) {
        fprintf(stderr, "%s returned MQCC_FAILED\n", call);
    } else if (CompCode == MQCC_UNKNOWN) {
        fprintf(stderr, "%s returned MQCC_UNKNOWN\n", call);
    } else {
        fprintf(stderr,
                "%s returned an unexpected CompCode (%d)\n",
                call,
                CompCode);
    }
    fprintf(stderr, "Reason is %d\n", Reason);
}
