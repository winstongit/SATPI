/* StreamClient.cpp

   Copyright (C) 2015 - 2017 Marc Postema (mpostema09 -at- gmail.com)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
   Or, point your browser to http://www.gnu.org/copyleft/gpl.html
*/
#include <StreamClient.h>

#include <Log.h>
#include <socket/SocketClient.h>
#include <Stream.h>

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#include <ctime>

	StreamClient::StreamClient() :
			_httpc(nullptr),
			_streamID(-1),
			_clientID(-1),
			_sessionID("-1"),
			_watchdog(0),
			_sessionTimeout(60),
			_cseq(0),
			_canClose(false) {}

	StreamClient::~StreamClient() {}

	void StreamClient::setSessionCanClose(bool close) {
		base::MutexLock lock(_mutex);

		SI_LOG_DEBUG("Stream: %d, Session can close: %d", _streamID, close);
		_canClose = close;
	}

	void StreamClient::teardown(bool gracefull) {
		base::MutexLock lock(_mutex);

		_watchdog = 0;
		_canClose = true;
		if (!gracefull) {
			_sessionID = "-1";
		}
		_httpc = nullptr;
	}

	void StreamClient::restartWatchDog() {
		base::MutexLock lock(_mutex);

		// reset watchdog and give some extra timeout
		_watchdog = std::time(nullptr) + _sessionTimeout + 15;
	}

	void StreamClient::selfDestruct() {
		base::MutexLock lock(_mutex);
		_watchdog = 1;
	}

	bool StreamClient::isSelfDestructing() const {
		base::MutexLock lock(_mutex);
		return _watchdog == 1;
	}

	bool StreamClient::sessionTimeout() const {
		base::MutexLock lock(_mutex);

		return (((_httpc == nullptr) ? -1 : _httpc->getFD()) == -1) &&
			   (_watchdog != 0) &&
			   (_watchdog < std::time(nullptr));
	}

	void StreamClient::setSocketClient(SocketClient &socket) {
		base::MutexLock lock(_mutex);
		_httpc = &socket;
	}

	SocketAttr &StreamClient::getRtpSocketAttr() {
		return _rtp;
	}

	SocketAttr &StreamClient::getRtcpSocketAttr() {
		return _rtcp;
	}

	// =======================================================================
	//  -- HTTP member functions ---------------------------------------------
	// =======================================================================

	bool StreamClient::sendHttpData(const void *buf, std::size_t len, int flags) {
		base::MutexLock lock(_mutex);
		return (_httpc == nullptr) ? false : _httpc->sendData(buf, len, flags);
	}

	bool StreamClient::writeHttpData(const struct iovec *iov, int iovcnt) {
		base::MutexLock lock(_mutex);
		return (_httpc == nullptr) ? false : _httpc->writeData(iov, iovcnt);
	}

	int StreamClient::getHttpSocketPort() const {
		base::MutexLock lock(_mutex);
		return (_httpc == nullptr) ? 0 : _httpc->getSocketPort();
	}

	int StreamClient::getHttpNetworkSendBufferSize() const {
		base::MutexLock lock(_mutex);
		return (_httpc == nullptr) ? 0 : _httpc->getNetworkSendBufferSize();
	}

	bool StreamClient::setHttpNetworkSendBufferSize(int size) {
		base::MutexLock lock(_mutex);
		return (_httpc == nullptr) ? false : _httpc->setNetworkSendBufferSize(size);
	}
