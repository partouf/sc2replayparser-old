
#include <Groundfloor/Atoms/GFInitialize.h>
#include <Groundfloor/Atoms/GFDefines.h>
#include <Groundfloor/Materials/GFGarbageCollector.h>
#include <Groundfloor/Materials/GFFunctions.h>
#include <Groundfloor/Bookshelfs/GFBFunctions.h>

#include <MySQLBooks/MySQLSquirrel.h>

#ifdef _MSC_VER
   #include <SFmpq_static.h>
#else
   #include <StormLib.h>
#endif


#include "detailsparser.h"


CDetailsParser *parser;


bool MapMpqFile( HANDLE mpq, const char *name, TGFString *data ) {
  HANDLE file;
  if ( SFileOpenFileEx( mpq, name, 0, &file ) == 0 ) {
     return false;
  }
  unsigned int sz,ss;
  sz = SFileGetFileSize( file, 0 );
  data->setLength( sz );

  if ( sz == 0 ) {
     return false;
  }
  ss = SFileReadFile( file, data->getValue(), sz, 0, 0 );
  SFileCloseFile( file );
  if ( ss == 0 ) {
    return false;
  };
  return true;
};

void addMetaData( TMySQLSquirrelConnection *conn, int iFileId, const char *key, const TGFString *value ) {
   TGFString qryInsert("insert into reffilemetadata (`id`,`key`,`value`) values (:id,:key,:value)");
   TGFString qryUpdate("update reffilemetadata set `value`=:value where `id`=:id and `key`=:key");
   TMySQLSquirrel qry( conn );
   TSquirrelReturnData errData;

   qry.setQuery( &qryInsert );
   qry.findOrAddParam("id")->setInteger( iFileId );
   qry.findOrAddParam("key")->setString( key );
   qry.findOrAddParam("value")->setString( value );
   if ( qry.open( &errData ) ) {
      qry.close();
   } else {
      qry.setQuery( &qryUpdate );
      if ( qry.open( &errData ) ) {
         qry.close();
      } else {
         printf( "mysqlerror: %s\n", errData.errorstring.getValue() );
      }
   }
}

void updateMetaData( TMySQLSquirrelConnection *conn, int iFileId ) {
   TGFString *t = parser->createTeamStr( &(parser->lstPlayers_team1) );
   addMetaData( conn, iFileId, "team1", t );
   delete t;

   t = parser->createTeamStr( &(parser->lstPlayers_team2) );
   addMetaData( conn, iFileId, "team2", t );
   delete t;

   t = parser->createTeamStr( &(parser->lstPlayers_other) );
   addMetaData( conn, iFileId, "other", t );
   delete t;

   addMetaData( conn, iFileId, "map", parser->mapname.link() );
}

int main( int argc, char ** argv ) {
   TGFString mpqfile;
   if ( initGroundfloor() ) {
      if ( initMySQLBooks() ) {
         TMySQLSquirrelConnection conn;
         conn.host.set("127.0.0.1");
         conn.username.set("yafbox");
         conn.password.set("");
         conn.connect();
         conn.selectDatabase("yafbox");

         int iFileId = 0;
         if ( argc > 2 ) {
            mpqfile.setValue_ansi( argv[1] );
            TGFBValue v;
            v.setString( argv[2] );
            iFileId = v.asInteger();
         } else {
            printf( "Usage: sc2replayparser <fullpath> <fileid>\n" );
            return 2;
//            iFileId = 115;
//            mpqfile.setValue_ansi( "D:\\Scrap Station (2).SC2Replay" );
         }

         if ( iFileId == 0 ) {
            printf("invalid fileid\n");
            return 2;
         }
         
         if ( !GFFileExists(&mpqfile) ) {
            printf("invalid path\n");
            return 2;
         }

         HANDLE mpq;
      
         if ( SFileOpenArchive( mpqfile.getValue(), 0, 0, &mpq ) ) {
         } else {
            printf("FAIL");
            return 2;
         }

         TGFString *replaydata = new TGFString();
         if ( !MapMpqFile( mpq, "replay.details", replaydata ) ) {
            printf("Error: no file - replay.details\n");
            SFileCloseArchive(mpq);
            return 2;
         };

         SFileCloseArchive(mpq);

         parser = new CDetailsParser();
         parser->loadFromString( replaydata );

         parser->printdebug();
         updateMetaData( &conn, iFileId );
         delete parser;

         delete replaydata;
         
         finiMySQLBooks();
      }

      finiGroundfloor();
   }
   //system("pause");
//   _CrtDumpMemoryLeaks();
   printf( "Done\n" );

   return 1;
}

