/*
 * (C) Copyright IBM Corporation 2020
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

var http = require('http');
const port = process.env.NODE_PORT || 3000;
const address = process.env.NODE_ADDRESS || '127.0.0.1';

var execSync = require('child_process').execSync;

var dspmq_stdout;

http.createServer(function (req, res) {
    var qmname = req.url.substr(1);
    if (qmname) {
        var result = '';
        var response = 404;

        try {
            dspmq_stdout = execSync('/home/rdqmadmin/mqazureqmstatus ' + qmname);
            response = 200;
        } catch (err) {
            dspmq_stdout = '';
        }
    } else {
        console.log('No queue manager name supplied\n');
    }
    //console.log('response for queue manager ' + qmname + ' is ' + response)
    // Uncomment the line above if you need to check what response the health probe is returning
    // You can see the output by running "sudo journalctl --no-pager _SYSTEMD_UNIT=mq_probe.service"
    res.writeHead(response, {'Content-Type': 'text/plain'});
    res.end(dspmq_stdout.toString() + '\n');
}).listen(port, address);
console.log('Server running at http://' + address + ':' + port + '/');
