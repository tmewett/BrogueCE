//
//  GameCenterIOS4.m
//  GKTapper
//
//  Created by Seth Howard on 8/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GameCenterIOS4.h"

@interface GameCenterIOS4()
- (void)loadUnhandledAchievements;
- (void)handleUnhandledEarnedAchievements;
- (void)saveUnhandledAchievements;

- (void)cachScore:(double)score withCategory:(NSString *)category;
- (void)saveUnhandledScores;
- (void)loadUnhandledScores;
- (void)clearUnhandledScores;
- (void)handleUnhandledScores;

@property(nonatomic, retain) NSMutableArray *unhandledEarnedAchievements;
@property(nonatomic, retain) NSMutableArray *unhandledScores;
@end

@implementation GameCenterIOS4
@synthesize unhandledScores = _unhandledScores, unhandledEarnedAchievements = _unhandledEarnedAchievements;

- (id)init
{
    self = [super init];
    if (self) {
        _unhandledScores = [[NSMutableArray alloc] initWithCapacity:0];
        _unhandledEarnedAchievements = [[NSMutableArray alloc] initWithCapacity:0];
    }
    
    return self;
}

- (void)dealloc {
    [_unhandledEarnedAchievements release];
    [_unhandledScores release];
    [super dealloc];
}

#pragma mark -
#pragma Authentication

//TODO: let's be a bit careful here... we are duplicating code and not calling the parent
//~~~ OVERRIDE
- (void)authenticateLocalUser{
	if([GKLocalPlayer localPlayer].authenticated == NO){
		[[GKLocalPlayer localPlayer] authenticateWithCompletionHandler:^(NSError *error) {
            //load any left over achievements that may have never been sent out
            [self loadUnhandledAchievements];
            [self loadUnhandledScores];
            
			if (error == nil) {
                //insert code here to handle a successful auth
                _gameCenterFeaturesEnabled = TRUE;
				_loginCount = 0;

                //make sure there are not any unhandledachievements pending
                [self handleUnhandledEarnedAchievements];
                [self handleUnhandledScores];
				
				//Submit the played game achievement
				[self submitAchievement:@"mypetzombie.playedgame" percentComplete:100];
            }
            else {
                //your app can process the error
                NSLog(@"game center auth error: %@", error);
                _gameCenterFeaturesEnabled = FALSE;
				_loginCount++;
                
				if (_loginCount < 3) {

					UIAlertView* alert = [[[UIAlertView alloc] initWithTitle: @"Game Center Account Required" message:[NSString stringWithFormat: @"Reason: %@", [error localizedDescription]] delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Try Again", nil] autorelease];
					[alert show];
				}
            }
		}];
	}
}

#pragma mark -
#pragma Achievements

//create custom class before calling parent
- (void)submitAchievement:(NSString *)identifier percentComplete:(double)percentComplete withCompletionHandler:(void (^)(GKAchievement *, NSError *))completionHandler{
    
    void (^gcLegacyCompletionHandler)(GKAchievement *, NSError *);
    
    gcLegacyCompletionHandler = ^(GKAchievement *achievement, NSError *error) {
        //TODO: cache achievement... if achievement is nil create on before caching
        if (error) {
            if (achievement == nil) {
                achievement = [[[GKAchievement alloc] initWithIdentifier:identifier] autorelease];
                achievement.percentComplete = percentComplete;
            }
            
            [_unhandledEarnedAchievements addObject:achievement];
            [self saveUnhandledAchievements];
        }
        
        if (completionHandler) {
            completionHandler(achievement, error);
        }
    };
    
    [super submitAchievement:identifier percentComplete:percentComplete withCompletionHandler:gcLegacyCompletionHandler];
}

#pragma mark -
#pragma Scores

- (void)reportScore:(int64_t)score forCategory:(NSString *)category withCompletionHandler:(void (^)(NSError *))completionHandler{
    void (^gcLegacyCompletionHandler)(NSError *);
    
    gcLegacyCompletionHandler = ^(NSError *error) {
        //TODO: cache achievement... if achievement is nil create on before caching
        if (error) {
            [self cachScore:score withCategory:category];
            [self saveUnhandledScores];
        }
        
        if (completionHandler) {
            completionHandler(error);
        }
    };
    
    [super reportScore:score forCategory:category withCompletionHandler:gcLegacyCompletionHandler];
}

#pragma mark -
#pragma Achievement Caching

//saves for a player id just in case they log off or leave before we regain a network connection
- (void)saveUnhandledAchievements{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *savePath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"unhandledAchievements"];
	[NSKeyedArchiver archiveRootObject:_unhandledEarnedAchievements toFile:savePath];
}

//loads for a player id if there are any waiting
- (void)loadUnhandledAchievements{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *savePath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"unhandledAchievements"];
	
	self.unhandledEarnedAchievements = [NSKeyedUnarchiver unarchiveObjectWithFile:savePath];
    
	if (!_unhandledEarnedAchievements) {
		self.unhandledEarnedAchievements = [[[NSMutableArray alloc] initWithCapacity:0] autorelease];
	}
}

- (void)clearUnhandledAchievements{
	[_unhandledEarnedAchievements removeAllObjects];
	[self saveUnhandledAchievements];
}

//we may not of had an internet connection or lost it momentarily
- (void)handleUnhandledEarnedAchievements{
    //TODO: check for a network connection so we can save some cycles
	if (self.gameCenterFeaturesEnabled && [_unhandledEarnedAchievements count]) {
		NSArray *unhandledAchievements = [_unhandledEarnedAchievements copy];
        //	[_unhandledEarnedAchievements removeAllObjects];
		[self clearUnhandledAchievements];
		
		for(GKAchievement *achievement in unhandledAchievements){
			//[self reportAchievementIdentifier:achievement.identifier percentComplete:achievement.percentComplete];
            [self submitAchievement:achievement.identifier percentComplete:achievement.percentComplete];
		}
		
		[unhandledAchievements release];
	}
	else {
		if (self.gameCenterFeaturesEnabled) {
			[self performSelector:@selector(handleUnhandledEarnedAchievements) withObject:nil afterDelay:120.0];
		}
	}
}

#pragma mark -
#pragma Score Caching

- (void)cachScore:(double)score withCategory:(NSString *)category{
   /* NSMutableDictionary *scoreDict = [NSMutableDictionary dictionaryWithCapacity:1];
    [scoreDict setObject:category forKey:@"category"];
    [scoreDict setObject:[NSNumber numberWithDouble:score] forKey:@"score"];
    [_unhandledScores addObject:scoreDict];*/
    GKScore *_score = [[GKScore alloc] initWithCategory:category];
    _score.value = score;
    [_unhandledScores addObject:_score];
    [_score release];
}

//saves for a player id just in case they log off or leave before we regain a network connection
- (void)saveUnhandledScores{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *savePath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"unhandledScores"];
	[NSKeyedArchiver archiveRootObject:_unhandledScores toFile:savePath];
}

//loads for a player id if there are any waiting
- (void)loadUnhandledScores{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *savePath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"unhandledScores"];
	
	self.unhandledScores = [NSKeyedUnarchiver unarchiveObjectWithFile:savePath];
    
	if (!_unhandledScores) {
		self.unhandledScores = [[[NSMutableArray alloc] initWithCapacity:0] autorelease];
	}
}

- (void)clearUnhandledScores{
	[_unhandledScores removeAllObjects];
	[self saveUnhandledScores];
}

//we may not of had an internet connection or lost it momentarily
- (void)handleUnhandledScores{
    //TODO: check for a network connection so we can save some cycles
	if (self.gameCenterFeaturesEnabled && [_unhandledScores count]) {
		NSArray *unhandledScores = [_unhandledScores copy];
		[self clearUnhandledScores];
		
		for(GKScore *score in unhandledScores){
            [self reportScore:score.value forCategory:score.category];
		}
		
		[unhandledScores release];
	}
	else {
		if (self.gameCenterFeaturesEnabled) {
			[self performSelector:@selector(handleUnhandledScores) withObject:nil afterDelay:120.0];
		}
	}
}

@end
