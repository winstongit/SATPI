/* StreamClient.h

   Copyright (C) 2015, 2016 Marc Postema (mpostema09 -at- gmail.com)

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
#ifndef STREAM_CLIENT_H_INCLUDE
#define STREAM_CLIENT_H_INCLUDE STREAM_CLIENT_H_INCLUDE

#include <FwDecl.h>
#include <socket/SocketAttr.h>
#include <socket/SocketClient.h>
#include <base/Mutex.h>

#include <string>

FW_DECL_NS0(Stream);

/// StreamClient defines the owner/participants of an stream
class StreamClient {
	public:
		// =======================================================================
		// Constructors and destructor
		// =======================================================================
		StreamClient();
		virtual ~StreamClient();

		///
		void setClientID(int id) {
			_clientID = id;
		}

		///
		void teardown(bool gracefull);

		///
		void restartWatchDog();

		///
		bool checkWatchDogTimeout();

		///
		void copySocketClientAttr(const SocketClient &socket);

		///
		const std::string getMessage() const {
			base::MutexLock lock(_mutex);
			return _httpcMsg;
		}

		///
		std::string getIPAddress() const {
			base::MutexLock lock(_mutex);
			return _ip_addr;
		}

		///
		std::string getSessionID() const {
			base::MutexLock lock(_mutex);
			return _sessionID;
		}

		///
		void setSessionID(const std::string &sessionID) {
			base::MutexLock lock(_mutex);
			_sessionID = sessionID;
		}

		///
		unsigned int getSessionTimeout() const {
			base::MutexLock lock(_mutex);
			return _sessionTimeout;
		}

		///
		int getHttpcFD() const {
			base::MutexLock lock(_mutex);
			return _httpcFD == nullptr ? -1 : *_httpcFD;
		}

		///
		void setCanClose(bool close);

		///
		bool canClose() const  {
			base::MutexLock lock(_mutex);
			return _canClose;
		}

		///
		void setCSeq(int cseq) {
			base::MutexLock lock(_mutex);
			_cseq = cseq;
		}

		///
		int getCSeq() const {
			base::MutexLock lock(_mutex);
			return _cseq;
		}

		///
		void setRtpSocketPort(int port);

		///
		int getRtpSocketPort() const;

		///
		void setRtcpSocketPort(int port);

		///
		int getRtcpSocketPort() const;

		///
		const struct sockaddr_in &getRtpSockAddr() const;

		///
		const struct sockaddr_in &getRtcpSockAddr() const;

		// =======================================================================
		// Data members
		// =======================================================================
	private:
		base::Mutex  _mutex;           ///
		int          _clientID;
		const int   *_httpcFD;         /// For sending reply to and checking
		                               /// connection
		std::string  _httpcMsg;        /// message from client
		std::string  _ip_addr;         /// IP address of client
		std::string  _sessionID;       ///
		time_t       _watchdog;        /// watchdog
		unsigned int _sessionTimeout;
		int          _cseq;            /// RTSP sequence number
		bool         _canClose;
		SocketAttr   _rtp;
		SocketAttr   _rtcp;
}; // class StreamClient

#endif // STREAM_CLIENT_H_INCLUDE