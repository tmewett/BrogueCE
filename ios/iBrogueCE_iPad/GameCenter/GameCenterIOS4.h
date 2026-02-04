//
//  GameCenterIOS4.h
//  GKTapper
//
//  Created by Seth Howard on 8/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

//pre iOS5 game center must support saving scores and achievements and resubmitting when offline

#import "GameCenterManager.h"

@interface GameCenterIOS4 : GameCenterManager{
    //encode this object.. save it out... do something with it so it persists and can be handled at a later time
	NSMutableArray *_unhandledEarnedAchievements;
    NSMutableArray *_unhandledScores;
}

@end
