// Copyright (C) 2004 Team Python
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "Server.h"
#include "Network.h"
#include "RealmListSrv.h"
#include "RedirectorSrv.h"
#include "WorldServer.h"
#include "Log.h"
#include "Threads.h"
#include "Database.h"

#include "stdio.h"
#include <iostream>
#include <time.h>


#if PLATFORM == PLATFORM_WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include <string>
using namespace::std;

#define SERVERIP "INSERT SERVER IP HERE"
#define LANIP "INSERT LAN IP HERE"
#define SERVERNAME "INSERT SERVER NAME HERE"

#define DBHOST "DB SERVER IP HERE"
#define DBUSER "DBUSERNAME"
#define DBPASS "DBPASSWORD"
#define DBDB "DB"

#ifndef VERSION
# define VERSION "0.10.2.0"
#endif
#if PLATFORM == PLATFORM_WIN32
# define FULLVERSION VERSION "-CVS-win"
#else
# define FULLVERSION VERSION "-CVS-*nix"
#endif

#define DEFAULT_LOOP_TIME 0 /* 0 millisecs - instant */

RedirectorSrv *mainredirector=0,*lanredirector=0, *devredirector=0, *dcheese_redirector=0, *rguy_redirector=0, *rguylinux_redirector=0;
std::string cHost, lHost, cName;
std::string DBhost,DBuser,DBpass,DBdb;

/* method to launch the server */


void launchWorld()
{
    string sTemp;

    Database::getSingleton( ).Initialise( DBhost.c_str(), DBuser.c_str(), DBpass.c_str(), DBdb.c_str() );

    WorldServer::getSingleton( ).Initialise( 8140, "World Server" );
    sTemp="";
    sTemp.append(cHost.c_str( ));
    sTemp.append(":8140");

    mainredirector = new RedirectorSrv( );
    mainredirector->Initialise( 9010, "Redirector Server", const_cast<char *>(sTemp.c_str())   );

    printf("Host: %s\n",sTemp.c_str());
    printf("Database: %s on %s:%s@%s\n",DBdb.c_str( ),DBuser.c_str(),DBpass.c_str(),DBhost.c_str() );
    mainredirector->Start( );

    WorldServer::getSingleton( ).Start();
    Threads::getSingleton( ).setCurrentPriority( Threads::TP_LOWER );

	WorldServer::getSingleton().LoadHelp();

    printf("Team python server started...\n");

	printf("World Logging is ");
	if (Network::getSingleton().IsLoggingWorld())
		printf("on.\n");
	else
		printf("off.\n");

	printf("Auto Account Creation is ");
	if (Database::getSingleton().isAutoCreateAccts())
		printf("on.\n");
	else
		printf("off.\n");
	
	printf("Firewall is ");
    if (Database::getSingleton().isFirewall())
		printf("on.\n");
	else
		printf("off.\n");
	
	printf("Test Login/IP is now ");
	if (Database::getSingleton().isTestIP())
		printf("on.\n");
	else
		printf("off.\n");

    //  memset(cTmpHost,NULL,sizeof(cTmpHost));
    WorldServer::getSingleton().SetMotd("Welcome to the WoW emulation test server!\nType !commands for a list of server commands");
}


void launchServer(RealmListSrv *realms)
{
    string sTemp, lTemp, nTemp, oTemp, pTemp;

    Database::getSingleton( ).Initialise( DBhost.c_str(), DBuser.c_str(), DBpass.c_str(), DBdb.c_str() );

    WorldServer::getSingleton( ).Initialise( 8129/*8129*/, "World Server" );

    //  RedirectorSrv mainredirector,devredirector;

    // build string
    sTemp="";
    sTemp.append(cHost.c_str( ));
    sTemp.append(":8129");

    lTemp="";
    lTemp.append(lHost.c_str( ));
    lTemp.append(":8129");

    printf("Host: %s\n",sTemp.c_str());
    printf("LanHost: %s\n",lTemp.c_str());
    printf("Database: %s on %s:%s@%s\n",DBdb.c_str( ),DBuser.c_str(),DBpass.c_str(),DBhost.c_str() );
    
    mainredirector = new RedirectorSrv( );
    devredirector = new RedirectorSrv( );
    lanredirector = new RedirectorSrv( );

    mainredirector->Initialise( 9003, "Redirector Server", const_cast<char *>(sTemp.c_str())   );
    devredirector->Initialise( 9004, "Redirector Server", "127.0.0.1:8129" );
    lanredirector->Initialise( 9005, "Redirector Server", const_cast<char *>(lTemp.c_str())   );

    mainredirector->Start( );
    devredirector->Start( );
    lanredirector->Start( );

    realms->Initialise(3724, "RealmList Server");
    realms->Start();

    WorldServer::getSingleton( ).Start();


    // build string
    sTemp="";
    sTemp.append(cHost.c_str( ));
    sTemp.append(":8129");
    
    // Lanhost fix - Torg
    lTemp="";
    lTemp.append(lHost.c_str( ));
    lTemp.append(":8129");

    nTemp="";
    oTemp="";
    pTemp="";
    nTemp.append(cName.c_str( ));
    oTemp.append(cName.c_str( ));
    pTemp.append(cName.c_str( ));
    oTemp.append("(localhost mode)");
    pTemp.append("(LAN mode)");

    // Format: Realm_Name, Realm_IP, Icon (0 = Normal, 1 = PVP), Color (0 = Yellow, 1 = Red)
    realms->addRealm( const_cast<char *>(nTemp.c_str()), const_cast<char *>(sTemp.c_str()), 0, 0 );
    realms->addRealm( const_cast<char *>(oTemp.c_str()), "127.0.0.1:8129", 0, 0 );
    realms->addRealm( const_cast<char *>(pTemp.c_str()), const_cast<char *>(lTemp.c_str()), 0, 0 );
   

    Threads::getSingleton( ).setCurrentPriority( Threads::TP_LOWER );

	WorldServer::getSingleton().LoadHelp();

    printf("Team python server started...\n");

	printf("World Logging is ");
	if (Network::getSingleton().IsLoggingWorld())
		printf("on.\n");
	else
		printf("off.\n");

	printf("Auto Account Creation is ");
	if (Database::getSingleton().isAutoCreateAccts())
		printf("on.\n");
	else
		printf("off.\n");
	
	printf("Firewall is ");
    if (Database::getSingleton().isFirewall())
		printf("on.\n");
	else
		printf("off.\n");

	printf("Test Login/IP is now ");
	if (Database::getSingleton().isTestIP())
		printf("on.\n");
	else
		printf("off.\n");
 

    //  memset(cTmpHost,NULL,sizeof(cTmpHost));
    WorldServer::getSingleton().SetMotd("Welcome to the Team Python test server!\nWrite .help for a list of Python server commands\nHave fun :)");
}

void set_black_color()
{
#if PLATFORM == PLATFORM_WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
        0);
#endif
}
void set_gray_color()
{
#if PLATFORM == PLATFORM_WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
        FOREGROUND_INTENSITY);
#endif
}
void set_white_color()
{
#if PLATFORM == PLATFORM_WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
        FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif
}
void set_red_color()
{
#if PLATFORM == PLATFORM_WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
        FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
}

void set_green_color()
{
#if PLATFORM == PLATFORM_WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
        FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif
}

/* method to print the options menu */
void printMenu()
{

    set_gray_color();
    printf("\n/------------------------------------------------------------------------\\\n");
    printf("|%-34s","      ** Team Python Server **");
    set_green_color();
    printf(" %-7s",FULLVERSION);
    set_gray_color();
    printf("  for build %-5i   |\n",EXPECTED_WOW_CLIENT_BUILD);

    printf("|"); set_white_color();
    printf("                                  Commands                              ");
    set_gray_color(); printf("|\n|");
    set_green_color();
    printf(" start            ");
    set_gray_color();
    printf("starts the server                                     |\n|");

    set_green_color();
    printf(" host [host]      ");
    set_gray_color();
    printf("Sets a new WAN Host                                   |\n|");

    set_green_color();
    printf(" lanhost [host]   ");
    set_gray_color();
    printf("Sets a new LAN Host                                   |\n|");

    set_green_color();
    printf(" name [name]      ");
    set_gray_color();
    printf("Sets a new Realm Server Name                          |\n|");

    set_green_color();
    printf(" db [host] [user] [pass] [database]");
    set_gray_color();
    printf("                                     |\n|");
    printf("                  ");
    printf("Sets a new database server                            |\n|");

    set_green_color();
    printf(" info             ");
    set_gray_color();
    printf("Show general server informations                      |\n|");

    set_green_color();
    printf(" motd             ");
    set_gray_color();
    printf("Display the current motd                              |\n|");

    set_green_color();
    printf(" motd [text]      ");
    set_gray_color();
    printf("Set the new motd (Write & for new lines)              |\n|");

    set_green_color();
    printf(" worldtext [text] ");
    set_gray_color();
    printf("Send a system message to all the players in the world |\n|");

    set_green_color();
    printf(" help             ");
    set_gray_color();
    printf("Show this help message                                |\n|");

    set_green_color();
    printf(" delay            ");
    set_gray_color();
    printf("show and/or change console delay                      |\n|");

    set_green_color();
    printf(" worldstart       ");
    set_gray_color();
    printf("start only the world server                           |\n|");

    set_green_color();
    printf(" wlog             ");
    set_gray_color();
    printf("log packets sent and recv'd by the world server       |\n|");

    set_green_color();
    printf(" autocreate       ");
    set_gray_color();
    printf("auto create new accounts                              |\n|");

	//START OF LINA FIREWALL V4.1
	set_green_color();
    printf(" fw/ban/allow/rmv ");
    set_gray_color();
    printf("toggle on/off (def off) - ban - allow - remove - IP   |\n|");

    set_green_color();
    printf(" testip           ");
    set_gray_color();
    printf("active the test login/ip for GM                       |\n|");
	//END OF LINA FIREWALL V4.1

    set_green_color();
    printf(" realmlist        ");
    set_gray_color();
    printf("prints the realmlist                                  |\n|");

	//START OF LINA SAVE SERVER COMMAND
    set_green_color();
    printf(" saveall          ");
    set_gray_color();
    printf("force to save all characters                          |\n|");
	//END OF LINA SAVE SERVER COMMAND      
	
	//START OF LINA HELP SERVER COMMAND
    set_green_color();
    printf(" reloadhelp       ");
    set_gray_color();
    printf("reload help from db to server                         |\n|");
	//END OF LINA HELP SERVER COMMAND  

	set_red_color(); printf(" exit"); set_gray_color(); printf("/");
    set_red_color(); printf("quit"); set_gray_color(); printf("/");
    set_red_color(); printf("stop");
    set_gray_color();
    printf("   Quit and shutdown server                              |\n");
    printf("\\------------------------------------------------------------------------/\n\n");

}



/* method to clear screen */
void clearScreen()
{
#if PLATFORM == PLATFORM_WIN32
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleScreenBufferInfo(hConsole, &csbi);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(hConsole, TEXT(' '), dwConSize,
        coordScreen, &cCharsWritten);
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize,
        coordScreen, &cCharsWritten);
    SetConsoleCursorPosition(hConsole, coordScreen);
#endif
}
int main( void ) {
   
    DWORD dwTime; // System-clock time
    DWORD dwLoopTime; // override default loop time
    bool bRunning;	// server running?

    dwLoopTime= DEFAULT_LOOP_TIME; // set real loop time to default one
    cHost     = SERVERIP; // set the host to default host
    lHost     = LANIP;
    cName     = SERVERNAME; 
    DBhost    = DBHOST;
    DBuser    = DBUSER;
    DBpass    = DBPASS;
    DBdb      = DBDB;
    bRunning	= false; // server is not running initially
	
	RealmListSrv * realms = RealmListSrv::getSingletonPtr( );

    printMenu();

    char data[512];
	for(int j = 0;j < 512;j++) data[j] = 0;

	FILE *conf;
	conf = fopen("WoWPython.conf","r");
	
    while( strncmp(data,"exit",4)!=0 && strncmp(data,"quit",4)!=0 && strncmp(data,"stop",4)!=0 )
    {
        // moved to server loop
        //if(bRunning){
        //  // Server is running, update the world stuff here
        //  WorldServer::getSingleton( ).Update();
        //}

        //if(!fgets(data, sizeof(data), stdin )) {
        //    Threads::getSingleton( ).setCurrentPriority( Threads::TP_IDLE );
        //    while(!fgets(data, sizeof(data), stdin ));
        //}
        //gets(data);

		if(conf == NULL || fgets(data,sizeof(data),conf) == NULL)
		{

	        if(!fgets(data, sizeof(data), stdin )) {
	            Threads::getSingleton( ).setCurrentPriority( Threads::TP_IDLE );
	            Threads::getSingleton( ).closeCurrentThread( );
	            while(!fgets(data, sizeof(data), stdin ));
	        }
		}

	    while( data[ strlen(data)-1 ] == 0x0d || data[ strlen(data)-1 ] == 0x0a )
            data[ strlen(data)-1 ] = 0x00;
			
        if( strncmp(data,"help",4)== 0)
        {
            clearScreen();
            printMenu();
        }

        else if(!(strncmp(data,"start",5)))
        {
            if(!bRunning)
            {
                launchServer(realms);
                bRunning=true;
            }
            else
            {
                printf("Server already running.\n");
            }
        }

        else if(!(strncmp(data,"worldstart",10)))
        {
            if(!bRunning)
            {
                launchWorld();
                bRunning=true;
            }
            else
            {
                printf("Server already running.\n");
            }
        }

        else if(!(strncmp(data,"host ",5)))
        {
            if( data[5] )
            {
                cHost = data+5;
                printf("Successfully set new host to: %s\n",cHost.c_str( ) );
            }
            else
            {
                printf("Please supply a valid host address.\n");
            }
		}
		else if(!(strncmp(data,"realmlist",9)))
		{
			realms->printRealms();
        }

        else if(!(strncmp(data,"name ",5)))
        {
            if( data[5] )
            {
                cName = data+5;
                printf("Successfully set new realm server name to: %s\n",cName.c_str( ) );
            }
            else
            {
                printf("Please supply a valid realm server name.\n");
            }
		}
		else if(!(strncmp(data,"realmlist",9)))
		{
			realms->printRealms();
        }

        else if(!(strncmp(data,"lanhost ",8)))
        {
            if( data[8] )
            {
                lHost = data+8;
                printf("Successfully set new lan host to: %s\n",lHost.c_str( ) );
            }
            else
            {
                printf("Please supply a valid host address.\n");
            }

        }

        else if(!(strncmp(data,"db ",3))) {
            strtok((char*)data, " ");
            char* host = strtok(NULL, " ");
            char* user = strtok(NULL, " ");
            char* pass = strtok(NULL, " ");
            char* db = strtok(NULL, " ");
            if( host ) DBhost = host;
            if( user ) DBuser = user;
            if( pass ) DBpass = pass;
            if( db )  DBdb = db;
        }


        else if(!(strncmp(data,"host",4)))
        {
            printf("Current host set to: %s\n",cHost.c_str( ) );
        }

        else if(!(strncmp(data,"lanhost",7)))
        {
            printf("Current lanhost set to: %s\n",lHost.c_str( ) );
        }

        else if(!(strncmp(data,"name",4)))
        {
            printf("Current realm name set to: %s\n",cName.c_str( ) );
        }

        else if(!(strncmp(data,"delay ",6)))
        {
            int dIndex=0;
            /* Increment dIndex until blank key */
            while((data[dIndex+6]!=0 && data[dIndex+6]!=' ')&& ++dIndex);
            /* Copy selected part of data over to data */
            memcpy(data,data+6,dIndex);
            data[dIndex]=0;
            /* if argument for delay is a number, convert it and set it
            to be new looptime */
            if(sscanf( data, "%d", &dwLoopTime )!=0 && dIndex>0)
                printf("Console delay successfully set to: %d\n",dwLoopTime);
            else
            {
                /* else, if argument for delay is not a valid number,
                give a simple error message. */
                printf("Error setting new Console delay value.\n");
            }

        }

        else if(!(strncmp(data,"delay",5)))
        {
            printf("Console delay currently set to: %i\n",dwLoopTime);
        }
        else if(strncmp(data,"worldtext ",10)==0)
        {
            int textsize;
            for( textsize=0; data[textsize+10]!=0; textsize++ )
            {
                if(data[textsize+10]=='&')
                    data[textsize+10]='\n';
            }
            memcpy(data,data+10,textsize);
            data[textsize]=0;
            WorldServer::getSingleton().SendWorldText((uint8*)data);
            printf("Message sent to all players:\n(%s)\n",data);
        }
        else if(strncmp(data,"info",4)==0)
        {
            printf("Users connected: %i\n",WorldServer::getSingleton().GetClientsConnected());
        }
        else if(strncmp(data,"motd ",5)==0)
        {
            int textsize;
            for( textsize=0; data[textsize+5]!=0; textsize++ )
            {
                if(data[textsize+5]=='&')
                    data[textsize+5]='\n';
            }
            memcpy(data,data+5,textsize);
            data[textsize]=0;
            WorldServer::getSingleton().SetMotd(data);
            printf("New motd has been set.\n");
        }
        else if(strncmp(data,"motd",4)==0)
        {
            printf("Current message of the day is:\n%s\n", WorldServer::getSingleton().GetMotd());
        }
        else if(strncmp(data,"exit",4)==0 || strncmp(data,"quit",4)==0 || strncmp(data,"stop",4)==0)
        {
            bRunning=false;
        }
        else if(strncmp(data,"time",4)==0)
        {
            printf("Game Time is: %d\n", WorldServer::getSingleton().updateGameTime());
        }
        else if(strncmp(data,"wlog",4)==0)
        {
            Network::getSingleton().toggleWorldLogging();
            printf("World Logging is now ");
            if (Network::getSingleton().IsLoggingWorld())
                printf("on.\n");
            else
                printf("off.\n");
        }
        else if(strncmp(data,"autocreate",10)==0)
        {
            // Toggle Auto Create Accounts
            Database::getSingleton().toggleAutoCreateAcct();
            printf("Auto Account Creation is now ");
            if (Database::getSingleton().isAutoCreateAccts())
                printf("on.\n");
            else
                printf("off.\n");
        }
		//START OF LINA FIREWALL SERVER COMMAND
		else if(strncmp(data,"fw",2)==0)
        {
            Database::getSingleton().toggleFirewall();
            printf("Firewall is now ");
            if (Database::getSingleton().isFirewall())
                printf("on.\n");
            else
                printf("off.\n");
        }
		else if(strncmp(data,"ban ",4)==0)
        {
			int textsize;
            for( textsize=0; data[textsize+4]!=0; textsize++ )
            {
                if(data[textsize+4]=='&')
                    data[textsize+4]='\n';
            }
            memcpy(data,data+4,textsize);
            data[textsize]=0;

			DatabaseInterface *dbi = Database::getSingleton().createDatabaseInterface( );
			dbi->BanIP((char*)data);
			Database::getSingleton( ).removeDatabaseInterface(dbi);
        }
		else if(strncmp(data,"allow ",6)==0)
        {
			int textsize;
            for( textsize=0; data[textsize+6]!=0; textsize++ )
            {
                if(data[textsize+6]=='&')
                    data[textsize+6]='\n';
            }
            memcpy(data,data+6,textsize);
            data[textsize]=0;

			DatabaseInterface *dbi = Database::getSingleton().createDatabaseInterface( );
			dbi->AllowIP((char*)data);
			Database::getSingleton( ).removeDatabaseInterface(dbi);
        }
		else if(strncmp(data,"rmv ",4)==0)
        {
			int textsize;
            for( textsize=0; data[textsize+4]!=0; textsize++ )
            {
                if(data[textsize+4]=='&')
                    data[textsize+4]='\n';
            }
            memcpy(data,data+4,textsize);
            data[textsize]=0;

			DatabaseInterface *dbi = Database::getSingleton().createDatabaseInterface( );
			dbi->RemoveIP((char*)data);
			Database::getSingleton( ).removeDatabaseInterface(dbi);
        }
		else if(strncmp(data,"testip",7)==0)
        {
            Database::getSingleton().toggleTestIP();
            printf("Test Login/IP is now ");
            if (Database::getSingleton().isTestIP())
                printf("on.\n");
            else
                printf("off.\n");
        }
		//END OF LINA FIREWALL SERVER COMMAND

		//START OF LINA SAVE SERVER COMMAND
		else if(strncmp(data,"saveall",7)==0)
        {
			if( WorldServer::getSingleton().GetClientsConnected() < 1 ) printf("No Character to save.\n");
			else printf("Characters saved: %i/%i\n", WorldServer::getSingleton().Save("Server"),WorldServer::getSingleton().GetClientsConnected());
        }
		//START OF LINA SAVE SERVER COMMAND
		//START OF LINA RELOAD HELP SERVER COMMAND
		else if(strncmp(data,"reloadhelp",10)==0)
        {
			WorldServer::getSingleton().LoadHelp();
			printf("Help reloaded");
		}
		//START OF LINA RELOAD HELP SERVER COMMAND

		else if(strncmp(data,"sm ",3)==0)
        {
			char* pGuid = strtok((char*)data, " ");
			uint32 Guid=(uint32)atoi(pGuid);

			Character * pChar = WorldServer::getSingleton().GetCharacter(Guid);
			if(pChar)
			{
				char* pOpcode = strtok(NULL, " ");
				uint32 Opcode=(uint32)atoi(pOpcode);

				char* pLength = strtok(NULL, " ");
				uint32 Length=(uint32)atoi(pLength);

				char* pData = strtok(NULL, " ");

				wowWData packets;
				packets.Initialise(Length, Opcode);
				packets << pData;
				pChar->pClient->SendMsg( &packets );
			}
			else printf("Unable to find Client with CharGuid: %i\n", Guid);
		}
        else
            printf("Unknown command (Type 'help' for a list of commands)\n");


        if(dwLoopTime>0)
        {
            dwTime=clock() * 1000 / CLOCKS_PER_SEC; // get current time
            while((clock() * 1000 / CLOCKS_PER_SEC - dwTime) < dwLoopTime); // 'idle' for dwLoopTime ms
        }
    }

    //Threads::getSingleton( ).setCurrentPriority( Threads::TP_LOWER );

    Log::getSingleton( ).outString( "Stopping realm list server..." );
    RealmListSrv::getSingleton( ).Stop( );

    if( mainredirector ) {
        Log::getSingleton( ).outString( "Stopping public redirector server..." );
        mainredirector->Stop( );
        delete mainredirector; mainredirector = 0;
    }

    if( devredirector ) {
        Log::getSingleton( ).outString( "Stopping developer redirector server..." );
        devredirector->Stop( );
        delete devredirector; devredirector = 0;
    }

    if( lanredirector ) {
        Log::getSingleton( ).outString( "Stopping LAN redirector server..." );
        lanredirector->Stop( );
        delete lanredirector; lanredirector = 0;
    }

    if( dcheese_redirector ) {
        Log::getSingleton( ).outString( "Stopping DeathCheese's redirector server..." );
        dcheese_redirector->Stop( );
        delete dcheese_redirector; dcheese_redirector = 0;
    }

    if( rguy_redirector ) {
        Log::getSingleton( ).outString( "Stopping RandomGuy's redirector server..." );
        rguy_redirector->Stop( );
        delete rguy_redirector; rguy_redirector = 0;
    }

    if( rguylinux_redirector ) {
        Log::getSingleton( ).outString( "Stopping RandomGuy's other redirector server..." );
        rguylinux_redirector->Stop( );
        delete rguylinux_redirector; rguylinux_redirector = 0;
    }

    Log::getSingleton( ).outString( "Stopping world server..." );
    WorldServer::getSingleton( ).Stop( );

    Log::getSingleton( ).outString( "Disconnecting clients..." );
    Network::getSingleton( ).disconnectAll( );

    // doesn't work...
    //Log::getSingleton( ).outString( "Stopping threads..." );
    //Threads::getSingleton( ).closeAllThreads( );

    Log::getSingleton( ).outString( "Halting process..." );
    Threads::getSingleton( ).closeCurrentThread( );
    return 0;
}

