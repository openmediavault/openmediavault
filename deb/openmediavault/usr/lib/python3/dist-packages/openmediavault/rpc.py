# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2020 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
import json
import socket
import struct

import openmediavault


class RpcException(Exception):
    def __init__(self, message, code=None, trace=None, http_status_code=None):
        super().__init__(message)
        self._code = code
        self._trace = trace
        self._http_status_code = http_status_code

    @property
    def code(self):
        return self._code

    @property
    def trace(self):
        return self._trace


def call(service, method, params=None):
    address = openmediavault.getenv("OMV_ENGINED_SO_ADDRESS")
    sndtimeo = openmediavault.getenv("OMV_ENGINED_SO_SNDTIMEO", type="int")
    rcvtimeo = openmediavault.getenv("OMV_ENGINED_SO_RCVTIMEO", type="int")
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.setsockopt(
        socket.SOL_SOCKET, socket.SO_SNDTIMEO, struct.pack("ll", sndtimeo, 0)
    )
    s.setsockopt(
        socket.SOL_SOCKET, socket.SO_RCVTIMEO, struct.pack("ll", rcvtimeo, 0)
    )
    try:
        s.connect(address)
    except socket.error as e:
        raise RuntimeError("Failed to connect {}: {}".format(address, e))
    request = json.dumps(
        {
            "service": service,
            "method": method,
            "params": params,
            "context": {"username": "admin", "role": 0x1},
        }
    )
    request = request + "\0"
    s.sendall(request.encode())
    chunks = []
    success = False
    while not success:
        chunk = s.recv(4096)
        if chunk == "":
            raise RuntimeError("Socket connection broken")
        if chunk.endswith(b"\0"):
            chunk = chunk[:-1]
            success = True
        chunks.append(chunk.decode())
    response = json.loads("".join(chunks))
    if response["error"] is not None:
        print(response)
        raise RpcException(**response["error"])
    return response["response"]
