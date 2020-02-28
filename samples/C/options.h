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

extern long int batchSize;
extern long int numberOfBatches;
extern long int sleepSeconds;
extern long int verbosity;
extern MQCHAR48 QMgrName;
extern size_t QMgrNameLen;
extern MQCHAR48 QName;
extern size_t QNameLen;
extern char UserId[13];
extern size_t UserIdLen;
extern char Password[MQ_CSP_PASSWORD_LENGTH + 1];
extern size_t PasswordLen;
extern size_t messageSize;

extern int processOptions(int argc, char *argv[]);
