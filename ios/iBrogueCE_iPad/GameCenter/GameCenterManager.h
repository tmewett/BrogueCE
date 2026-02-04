//
//  GameCenterManager.h
//  GameCenterTest
//
//  Created by Seth Howard on 8/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

//TODO: handle what happens when we establish a network connection

#import <Foundation/Foundation.h>
#import <GameKit/GameKit.h>

//game specific defines
// Leaderboards
#define kBrogueHighScoreLeaderBoard @"iBrogue_High_Score"

// Achievements
#define kAchievementArchivist @"brogue_archivist"
#define kAchievementCompanion @"brogue_companion"
#define kAchievementDragonslayer @"brogue_dragonslayer"
#define kAchievementIndomitable @"brogue_indomitable"
#define kAchievementJellymancer @"brogue_jellymancer"
#define kAchievementMystic @"brogue_mystic"
#define kAchievementPacifist @"brogue_pacifist"
#define kAchievementPaladin @"brogue_paladin"
#define kAchievementPureMage @"brogue_pure_mage"
#define kAchievementPureWarrior @"pure_warrior"
#define kAchievementSpecialist @"brogue_specialist"

@class GKLeaderboard, GKAchievement, GKPlayer;
@interface GameCenterManager : NSObject {
	NSMutableDictionary* earnedAchievementCache;
    BOOL _gameCenterFeaturesEnabled;
	int _loginCount;
}

@property (nonatomic, readonly) GKPlayer *localPlayer;
@property (nonatomic, readonly) BOOL gameCenterFeaturesEnabled;

+ (BOOL)areNewgameCenterFeaturesSupported;
+ (id)createGameCenter;
+ (GameCenterManager *) sharedInstance;
+ (BOOL)isGameCenterAvailable;
+ (void)registerForGameCenterAuthenicationChangeNotification:(id)observer withSelector:(SEL)selector;

- (void)authenticateLocalUser;

- (void)reportScore: (int64_t) score forCategory: (NSString*) category;
- (void)reportScore:(int64_t)score forCategory:(NSString *)category withCompletionHandler:(void(^)(NSError *error))completionHandler;
- (void)reloadHighScoresForCategory:(NSString*)category withCompletionHandler:(void(^)(NSArray *scores, NSError *error))completionHandler;

- (void)submitAchievement: (NSString*) identifier percentComplete: (double) percentComplete;
- (void)submitAchievement:(NSString*)identifier percentComplete:(double)percentComplete withCompletionHandler:(void(^)(GKAchievement *achievement, NSError * error))completionHandler;
- (void)resetAchievements;
- (void)resetAchievements:(void(^)(NSError *error))completionHandler;

- (void)mapPlayerIDtoPlayer:(NSString *)playerID withCompletionHandler:(void(^)(NSArray *playerArray, NSError *error))completionHandler;

@end
