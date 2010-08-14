
#ifndef __DETAILSPARSER_H__
#define __DETAILSPARSER_H__

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>
#include <Groundfloor/Molecules/GFProperty.h>
#include <Groundfloor/Materials/GFFileCommunicator.h>
#include <Groundfloor/Molecules/GFStringVector.h>

class CDetailsParser: public TGFFreeable {
protected:
   TGFFileCommunicator filecom;
   TGFString data;
   int totalplayercount;

   void determinePlayerNameAndRace( const TGFString *playerblock, TGFString *player, TGFString *race );
   void determinePlayerBlocks( TGFStringVector *v );
   int determineTeam( const TGFString *playerblock );
   void determineMap( const TGFString *block, TGFString *mapname );
public:
   TGFStringProperty mapname;
   TGFVector lstPlayers_team1;
   TGFVector lstPlayers_team2;
   TGFVector lstPlayers_other;

   CDetailsParser();
   ~CDetailsParser();

   void load( const TGFString *sFile );
   void loadFromString( const TGFString *sFile );

   TGFString *createTeamStr( const TGFVector *v );
   void printdebug();

};

#endif // __DETAILSPARSER_H__