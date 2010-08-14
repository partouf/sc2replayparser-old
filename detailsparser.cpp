
#include "detailsparser.h"

#include <cmath>

CDetailsParser::CDetailsParser() : TGFFreeable() {
}

CDetailsParser::~CDetailsParser() {
}

void CDetailsParser::determinePlayerBlocks( TGFStringVector *v ) {
   TGFString *chunk;

   TGFString playerblockstart;
   playerblockstart.setSize( 4 );
   playerblockstart.append( 0x05 );
   playerblockstart.append( 0x12 );
   playerblockstart.append( char(0x00) );
   playerblockstart.append( 0x02 );

   TGFString mapblockstart;
   mapblockstart.setSize( 2 );
   mapblockstart.append( 0x02 );
   mapblockstart.append( 0x02 );

   int p1 = data.pos( 0, &playerblockstart );
   int p2 = 0;
   while ( p1 != -1 ) {
      p2 = data.pos( p1 + playerblockstart.getLength(), &playerblockstart );
      if ( p2 != -1 ) {
         chunk = new TGFString();
         chunk->setValue( data.getPointer(p1), p2 - p1 );
         v->addChunk( chunk );
      } else {
         int p3 = data.pos(p1, &mapblockstart );
         if ( p3 != -1 ) {
            chunk = new TGFString();
            chunk->setValue( data.getPointer(p1), p3 - p1 );
            v->addChunk( chunk );

            // last block is hopefully the map block
            chunk = new TGFString();
            chunk->setValue( data.getPointer(p3), data.getLength() - p3 );
            v->addChunk( chunk );
         } else {
            chunk = new TGFString();
            chunk->setValue( data.getPointer(p1), data.getLength() - p1 );
            v->addChunk( chunk );
         }
      }
      
      p1 = data.pos( p1 + playerblockstart.getLength(), &playerblockstart );
   }
}

void CDetailsParser::determinePlayerNameAndRace( const TGFString *playerblock, TGFString *player, TGFString *race ) {
   char *s = playerblock->getValue();
   unsigned int ilen = floorl(s[4] / 2);
   player->setValue( playerblock->getPointer(5), ilen );

   if ( playerblock->pos_ansi( 5 + ilen, "Terran" ) != -1 ) {
      race->setValue_ansi("Terran");
   } else if ( playerblock->pos_ansi( 5 + ilen, "Protoss" ) != -1 ) {
      race->setValue_ansi("Protoss");
   } else if ( playerblock->pos_ansi( 5 + ilen, "Zerg" ) != -1 ) {
      race->setValue_ansi("Zerg");
   }
}

int CDetailsParser::determineTeam( const TGFString *playerblock ) {
   char *s = playerblock->getValue();
   BYTE i = s[playerblock->getLength()-1];

/*
if ( i != 0x00 ) {
      return floorl(i / 2);
   } else {
*/
   // determine by ammount of players /2
      int iTeamsize = floorl(totalplayercount / 2);
      if ( lstPlayers_team1.size() < iTeamsize ) {
         return 1;
      } else if ( lstPlayers_team2.size() < iTeamsize ) {
         return 2;
      }
//   }

   return 0;
}

void CDetailsParser::determineMap( const TGFString *block, TGFString *mapname ) {
   char *s = block->getValue();
   unsigned int ilen = floorl(s[2] / 2);
   mapname->setValue( block->getPointer(3), ilen );
}

void CDetailsParser::loadFromString( const TGFString *sData ) {
   if ( sData != NULL ) {
      data.setValue( sData );
   }

   TGFStringVector *v = new TGFStringVector();
   determinePlayerBlocks( v );

   for ( unsigned int i = 0; i < v->size() - 1; i++ ) {
      TGFString *block = static_cast<TGFString *>( v->elementAt(i) );
      TGFString *player = new TGFString();
      TGFString *race = new TGFString();
      determinePlayerNameAndRace( block, player, race );
      int iTeam = determineTeam( block );
      TGFString *c = new TGFString(player);
      c->append_ansi(".");
      c->append(race);
      if ( iTeam == 1 ) {
         lstPlayers_team1.addElement( c );
      } else if ( iTeam == 2 ) {
         lstPlayers_team2.addElement( c );
      } else {
         lstPlayers_other.addElement( c );
      }
      delete race;
      delete player;
   }

   determineMap( static_cast<TGFString *>(v->elementAt(v->size() - 1)), mapname.link() ); 

   delete v;
}

void CDetailsParser::load( const TGFString *sFile ) {
   data.setLength( 0 );

   TGFString sBuffer;
   TGFCommReturnData errData;

   sBuffer.setLength( 1024 );

   filecom.filename.set( sFile->getValue() );
   
   if ( filecom.connect() ) {
      while ( filecom.receive( &sBuffer, &errData ) ) {
         data.append( &sBuffer );
         if ( errData.eof ) {
            break;
         }
      }
      filecom.disconnect();
   }

   loadFromString( NULL );
}

void CDetailsParser::printdebug() {
   unsigned int i;
   
   TGFString * s = createTeamStr( &lstPlayers_team1 );
   printf( "Team 1 - %s\n", s->getValue() );
   delete s;
   s = createTeamStr( &lstPlayers_team2 );
   printf( "Team 2 - %s\n", s->getValue() );
   delete s;
   s = createTeamStr( &lstPlayers_other );
   printf( "Other - %s\n", s->getValue() );
   delete s;

   printf( "Map - %s\n", mapname.get() );
}

TGFString *CDetailsParser::createTeamStr( const TGFVector *v ) {
   TGFString *s = new TGFString();

   unsigned int i;
   for ( i = 0; i < v->size(); i++ ) {
      if ( s->getLength() != 0 ) {
         s->append_ansi(";");
      }
      s->append( static_cast<TGFString *>(v->elementAt(i)) );
   }

   return s;
}
