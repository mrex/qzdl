/*
 * This file is part of qZDL
 * Copyright (C) 2007-2010  Cody Harris
 * 
 * qZDL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <QMessageBox>
#include "ZDLUpdater.h"
#include "ZDLConfigurationManager.h"
#include "ZDLInfoBar.h"
#include <iostream>
using namespace std;

ZDLUpdater::ZDLUpdater(){
	http = new QHttp(this);
	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
			this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
	connect(http, SIGNAL(requestFinished(int, bool)),
			this, SLOT(httpRequestFinished(int, bool)));
	connect(http, SIGNAL(readyRead(const QHttpResponseHeader &)),
			this, SLOT(readyRead(const QHttpResponseHeader &)));
	host = "update.vectec.net";
	errorCode = 0;
	port = 80;
	updateCode = 0;
	httpGetId = 0;
}

ZDLUpdater::~ZDLUpdater(){
	delete http;
}

void ZDLUpdater::setHost(const char* host, const int port){
	this->host = host;
	this->port = port;
}

int ZDLUpdater::hasError(){
	return errorCode;
}

int ZDLUpdater::hasUpdate(){
	//cout << "hasUpdate" << endl;
	return updateCode;
}

void ZDLUpdater::updatesDisabledInfobar(){
	QMessageBox::warning(NULL,"Updates Disabled", "You have disabled checking for updates.  This means that you will no longer get notification of new releases.\n\nIf you did not disable the updates yourself, then " ZDL_ENGINE_NAME " has automatically disabled updates silently if there were connectivity issues with the update server.\n\nTo re-enable updates, please go to the settings tab in the main interface.",QMessageBox::Ok,QMessageBox::Ok);
}

void ZDLUpdater::updatesOldSystem(){
	QMessageBox::warning(NULL,"Old Update System", "Thank you for trying " ZDL_ENGINE_NAME "!!\n\nCurrently updates are pushed by a rather limited script located on our server.  Unfortunately, it doesn't quite have the ability to check to see if your version is the most current version.  You most likely checked out our version from SVN from our sourceforge website.  We only bump the version number on major releases, so we encourage you to manually check for updates on our sourceforge project page.\n\nhttp://sf.net/projects/ZDLSharp",QMessageBox::Ok,QMessageBox::Ok);
}

void ZDLUpdater::fetch(){
	fetch(0);
}

void ZDLUpdater::fetch(int doAnyways){
	//cout << "fetch" << endl;
	ZDLConf *zconf = ZDLConfigurationManager::getActiveConfiguration();
	ZDLSection *section = zconf->getSection("zdl.net");
	if (section){
		vector <ZDLLine*> fileVctr;
		section->getRegex("^updateManager$", fileVctr);
		
		for(unsigned int i = 0; i < fileVctr.size(); i++){
			if (strcmp(fileVctr[i]->getValue(), "disabled") == 0){
				ZDLConfigurationManager::setInfobarMessage("Updates are disabled.",1);
				ZDLInfoBar *bar = (ZDLInfoBar*)ZDLConfigurationManager::getInfobar();
				connect(bar,SIGNAL(moreclicked()),this,SLOT(updatesDisabledInfobar()));
				if(doAnyways == 0){
					return;
				}
			}
		}
		
	}
	
	if (httpGetId == 0){
		buffer.clear();
		updateCode = 0;
		http->setHost(this->host, QHttp::ConnectionModeHttp, this->port);
#ifdef Q_WS_WIN
		QString url = "/check.php?name=qzdl-win32-beta&id=";
#else
		QString url = "/check.php?name=qzdl-svn&id=";
#endif
		url += QString::number(ZDL_VERSION_ID);
		QHttpRequestHeader qreq("GET", url);
		QString ua = ZDL_ENGINE_NAME;
		ua += " ";
		ua += ZDL_VERSION_STRING;
		ua += " (";
#ifdef Q_WS_WIN
		if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based){
			ua += "Win32; ";
			if (QSysInfo::WindowsVersion == QSysInfo::WV_32s){
				ua += "Windows 3.1";
			}else if (QSysInfo::WindowsVersion == QSysInfo::WV_95){
				ua += "Windows 95";
			}else if (QSysInfo::WindowsVersion == QSysInfo::WV_98){
				ua += "Windows 98";
			}else if (QSysInfo::WindowsVersion == QSysInfo::WV_Me){
				ua += "Windows ME";
			}else {
				ua += "Windows Unknown";
			}
		}else if (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based){
			ua += "WinNT; ";
			if (QSysInfo::WindowsVersion == QSysInfo::WV_NT){
				ua += "Windows NT";
			}else if (QSysInfo::WindowsVersion == QSysInfo::WV_2000){
				ua += "Windows 2000";
			}else if (QSysInfo::WindowsVersion == QSysInfo::WV_XP){
				ua += "Windows XP";
			}else if (QSysInfo::WindowsVersion == QSysInfo::WV_2003){
				ua += "Windows Server 2003";
			}else if (QSysInfo::WindowsVersion == QSysInfo::WV_VISTA){
				ua += "Windows Vista";
			}else{
				ua += "Windows Unknown";
			}
				
		}else if (QSysInfo::WindowsVersion & QSysInfo::WV_CE_based){
			if (QSysInfo::WindowsVersion == QSysInfo::WV_CE){
				ua += "Windows CE";
			}else if (QSysInfo::WindowsVersion == QSysInfo::WV_CENET){
				ua += "Windows CS .NET";
			}else{
				ua += "Windows Unknown";
			}
			
			ua += "WinCE; ";
		}else{
			ua += "WinUnknown";
		}
#else
#ifdef Q_WS_MAC 
		if (QSysInfo::MacintoshVersion == QSysInfo::MV_9){
			ua += "MacOS9";
		}else if (QSysInfo::MacintoshVersion == QSysInfo::MV_10_0){
			ua += "MacOSX 10.0 Cheetah";
		}else if (QSysInfo::MacintoshVersion == QSysInfo::MV_10_1){
			ua += "MacOSX 10.1 Puma";
		}else if (QSysInfo::MacintoshVersion == QSysInfo::MV_10_2){
			ua += "MacOSX 10.2 Jaguar";
		}else if (QSysInfo::MacintoshVersion == QSysInfo::MV_10_3){
			ua += "MacOSX 10.3 Panther";
		}else if (QSysInfo::MacintoshVersion == QSysInfo::MV_10_4){
			ua += "MacOSX 10.4 Tiger";
		}else if (QSysInfo::MacintoshVersion == QSysInfo::MV_10_5){
			ua += "MacOSX 10.5 Leopard";
		}else if (QSysInfo::MacintoshVersion == QSysInfo::MV_Unknown){
			ua += "Mac Unknown";
		}else{
			ua += "Mac Unknown";
		}
#else
			ua += "Linux ";
			QFile kernel("/proc/sys/kernel/osrelease");
			if (kernel.exists() && kernel.open(QIODevice::ReadOnly)){
				QString lkv = kernel.readLine();
				ua += lkv.section("\n", 0, 0);
			}else{
				ua += "Unknown";
			}
				
#endif
#endif
		ua += "; UID:";
		ua += ZDL_UID;
		ua += ")";
		
		//cout << "User agent:" << ua.toStdString() << endl;
		qreq.setValue("Host", this->host);
		qreq.setValue("User-Agent", ua);
		httpGetId = http->request(qreq);
		
	}
	
}

void ZDLUpdater::httpRequestFinished(int requestId, bool error){
	//cout << "httpRequestFinished" << endl;
	if (requestId != httpGetId)
		return;
	if (error){
		//cout << "error" << endl;
	}
	QString str(buffer);
	//cout << "Buffer " << str.toStdString() << endl;
	if (str == "MISMATCH"){
		//cout << "There is an update" << endl;
		updateCode = 1;
		errorCode = 0;
	}else if (str == "ERROR-SYNTAX"){
		updateCode = 0;
		errorCode = 1;
	}else if (str == "ERROR-NOID"){
		updateCode = 0;
		errorCode = 2;
	}else if (str == "OKAY"){
		updateCode = 0;
		errorCode = 0;
	}else{
		updateCode = 0;
		errorCode = 3;
	}
	//cout << "ErrorCode:" << errorCode << " UpdateCode:" << updateCode << endl;
	httpGetId = 0;
	emit updateReady();
}

void ZDLUpdater::readyRead ( const QHttpResponseHeader & resp ){
	//cout << "readyRead: " << resp.reasonPhrase().toStdString() << endl;
	QByteArray inBytes = http->readAll();
	buffer.append(inBytes);
}

void ZDLUpdater::readResponseHeader(const QHttpResponseHeader &responseHeader){
	//cout << "readResponseHeader" << endl;
	errorCode = responseHeader.statusCode();
	//cout << "Error code: " << errorCode << endl;
	
}