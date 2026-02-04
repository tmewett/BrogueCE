//
//  GameCenterManager.m
//  GameCenterTest
//
//  Created by Seth Howard on 8/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

//TODO: look up KeyValue Data for syncing across multiple devices as well as saving game states - iCloud
//TODO: check into using player photos

/*
 [[[GKLocalPlayer localPlayer] loadPhotoForSize:GKPhotoSizeNormal with CompltionHandlier:^(UIImage *photo
 */


#import "GameCenterManager.h"
#import "GameCenterIOS4.h"
#import "AppDelegate.h"

#define kMaxScoresToLoad 25

static GameCenterManager *sharedGameCenterManager;

@interface GameCenterManager()
@property (retain) NSMutableDictionary* earnedAchievementCache;
@property (nonatomic, assign) BOOL checkingLocalPlayer;
@end

@implementation GameCenterManager

@synthesize earnedAchievementCache;
//@synthesize delegate;
@synthesize localPlayer;
@synthesize gameCenterFeaturesEnabled = _gameCenterFeaturesEnabled;

//recommended that initialization is created here and not by calling the class init.
//new features in iOS5 do not require us to save scores and acheivements when the user's 
//internet connection fails. We may also want to override any iOS5 specific calls like turn based
//calls... or throw a warning if this is a requirement

+ (BOOL)areNewgameCenterFeaturesSupported{
    NSString *reqSysVer = @"5.0";
	NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
	BOOL newGameCenterFeaturesSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
    
    return newGameCenterFeaturesSupported;
}
 
+ (id)createGameCenter{
    if ([GameCenterManager areNewgameCenterFeaturesSupported]) {
		sharedGameCenterManager = [[GameCenterManager alloc] init];
    }
    else
	{
		sharedGameCenterManager = [[GameCenterIOS4 alloc] init];
	}
    //else
    return sharedGameCenterManager;
}

+ (GameCenterManager *) sharedInstance
{
	@synchronized(sharedGameCenterManager)
	{
		if (!sharedGameCenterManager) {
			
			//Only create the sharedGameCenterManager object if gamecenter is supported
			if ([GameCenterManager isGameCenterAvailable]) {
				sharedGameCenterManager = [GameCenterManager createGameCenter];
			} else {
				//This line isn't strictly needed, but we'll put it here anyway
				sharedGameCenterManager = nil;
			}
		}
		
		return sharedGameCenterManager;
	}
}

- (id)init{
	self = [super init];
	if(self != nil){
		[GameCenterManager registerForGameCenterAuthenicationChangeNotification:self withSelector:@selector(authenticationChanged)];
       // [self authenticateLocalUser];
	}
	
	//NSLog(@"LeaderboardClassic: %@ kLeaderBasic: %@", kLeaderboardClassicPoints, kLeaderboardBasicPoints);
    
	return self;
}

- (void)dealloc{
	[earnedAchievementCache release];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

+ (BOOL)isGameCenterAvailable{
	// check for presence of GKLocalPlayer API
	Class gcClass = (NSClassFromString(@"GKLocalPlayer"));
	
	// check if the device is running iOS 4.1 or later
	NSString *reqSysVer = @"4.1";
	NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
	BOOL osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
	
	return (gcClass && osVersionSupported);
}

#pragma mark -
#pragma Authentication

+ (void)registerForGameCenterAuthenicationChangeNotification:(id)observer withSelector:(SEL)selector{
	[[NSNotificationCenter defaultCenter] addObserver:observer selector:selector name:GKPlayerAuthenticationDidChangeNotificationName object:nil];
}

- (void)finishGameCenterAuthWithError:(NSError *)error {
    if (error == nil) {
        //insert code here to handle a successful auth
        _gameCenterFeaturesEnabled = TRUE;
        _loginCount = 0;
    }
    else {
        //your app can process the error
        NSLog(@"game center auth error: %@", error);
        _gameCenterFeaturesEnabled = FALSE;
        _loginCount++;
        
        if (_loginCount < 3) {
       //     UIAlertView* alert = [[[UIAlertView alloc] initWithTitle: @"Game Center Account Required" message:[NSString stringWithFormat: @"Reason: %@", [error localizedDescription]] delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Try Again", nil] autorelease];
        //    [alert show];
        } else {
            //Still not logging in but let's not get stuck in a repeat loop
            NSLog(@"Not logging in repeat loop");
        }
    }
}

- (void)authenticateLocalUser{
    //return;
	NSLog(@"LocalPlayer: %@", [GKLocalPlayer localPlayer]);
    if (!self.checkingLocalPlayer) {
        self.checkingLocalPlayer = YES;
        GKLocalPlayer *thisPlayer = [GKLocalPlayer localPlayer];
        
        if (!thisPlayer.authenticated) {
            if (YES) {
                
                [thisPlayer setAuthenticateHandler:(^(UIViewController* aViewcontroller, NSError *error) {
                    
                    if (aViewcontroller) {
                      //  [self.delegate presentViewController:viewcontroller];
                        int64_t delayInSeconds = 1.0;
                        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
                        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
                            UIViewController *view = (UIViewController *)[(AppDelegate *)[[UIApplication sharedApplication] delegate] viewController];
                          
                            [view presentModalViewController:aViewcontroller animated:YES];
                        //    [[[viewController navigationController] topViewController] presentViewController:viewcontroller animated:YES completion:nil];
                        });
                        
                    } else {
                        [self finishGameCenterAuthWithError:error];
                    }
                    
                })];
                
            } else {
                
                [[GKLocalPlayer localPlayer]
                 authenticateWithCompletionHandler:^(NSError *error)
                 {
                     [self finishGameCenterAuthWithError:error];
                 }
                 ];
            }
            
        }
    }
    
    return;
    
	if([GKLocalPlayer localPlayer].authenticated == NO)
    {
		[[GKLocalPlayer localPlayer] authenticateWithCompletionHandler:^(NSError *error) {
			if (error == nil) {
                //insert code here to handle a successful auth
                _gameCenterFeaturesEnabled = TRUE;
				_loginCount = 0;
            }
            else {
                //your app can process the error
                NSLog(@"game center auth error: %@", error);
                _gameCenterFeaturesEnabled = FALSE;
				_loginCount++;
                
				if (_loginCount < 3) {
					UIAlertView* alert = [[[UIAlertView alloc] initWithTitle: @"Game Center Account Required" message:[NSString stringWithFormat: @"Reason: %@", [error localizedDescription]] delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Try Again", nil] autorelease];
					[alert show];
				} else {
					//Still not logging in but let's not get stuck in a repeat loop
					NSLog(@"Not logging in repeat loop");
				}
            }
		}];
	}
}


- (void)authenticationChanged{
	if ([GKLocalPlayer localPlayer].authenticated) {
		_gameCenterFeaturesEnabled = YES;
	}
	else {
		_gameCenterFeaturesEnabled = NO;
	}
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
    if (buttonIndex) {
        [self authenticateLocalUser];
    }	
}

#pragma mark -
#pragma Scores

- (void)reloadHighScoresForCategory:(NSString*)category withCompletionHandler:(void(^)(NSArray *scores, NSError *error))completionHandler {
	GKLeaderboard* leaderBoard = [[[GKLeaderboard alloc] init] autorelease];
	leaderBoard.category = category;
	leaderBoard.timeScope = GKLeaderboardTimeScopeAllTime;
	leaderBoard.range = NSMakeRange(1, kMaxScoresToLoad);
	
	[leaderBoard loadScoresWithCompletionHandler:  ^(NSArray *scores, NSError *error){
		if (completionHandler) {
            completionHandler(scores, error);
        }
        
        if (error) {
#if TARGET_IPHONE_SIMULATOR
            UIAlertView* alert = [[[UIAlertView alloc] initWithTitle: @"Error" message: [NSString stringWithFormat: @"reloadHighScoresForCategory: %@", [error localizedDescription]] delegate:self cancelButtonTitle:@"OK" otherButtonTitles:NULL] autorelease];
            [alert show];
#endif
        }
	}];
}


- (void)reportScore:(int64_t)score forCategory:(NSString*)category {
	[self reportScore:score forCategory:category withCompletionHandler:nil];
}

- (void)reportScore:(int64_t)score forCategory:(NSString *)category withCompletionHandler:(void(^)(NSError *error))completionHandler{
    GKScore *scoreReporter = [[[GKScore alloc] initWithCategory:category] autorelease];	
	scoreReporter.value = score;

    [scoreReporter reportScoreWithCompletionHandler: ^(NSError *error){
        if(error == nil){
            NSLog(@"ScoreReported");
        }
        else{
            //TODO: in cache score
            
#if TARGET_IPHONE_SIMULATOR
            UIAlertView* alert = [[[UIAlertView alloc] initWithTitle: @"Error" message: [NSString stringWithFormat: @"reportScore: %@", [error localizedDescription]] delegate:self cancelButtonTitle:@"OK" otherButtonTitles:NULL] autorelease];
            [alert show];
#endif
        }
        
        if (completionHandler) {
            completionHandler(error);
        }
    }];
}

#pragma mark -
#pragma Achievements

- (void)submitAchievement:(NSString*)identifier percentComplete:(double)percentComplete{
	[self submitAchievement:identifier percentComplete:percentComplete withCompletionHandler:nil];
}

//change this to getachievement... return an achievement and then report it in another method that we can overload.. this will
//take care of ios5 issues below
- (void)submitAchievement:(NSString*)identifier percentComplete:(double)percentComplete withCompletionHandler:(void(^)(GKAchievement *achievement, NSError * error))completionHandler{
	//GameCenter checks for duplicate achievements when the achievement is submitted, but if you only want to report 
	// new achievements to the user, then you need to check if it's been earned 
	// before you submit.  Otherwise you'll end up with a race condition between loadAchievementsWithCompletionHandler
	// and reportAchievementWithCompletionHandler.  To avoid this, we fetch the current achievement list once,
	// then cache it and keep it updated with any new achievements.
    if (!identifier) {
        return;
    }
    
	if(self.earnedAchievementCache == nil){
		[GKAchievement loadAchievementsWithCompletionHandler: ^(NSArray *scores, NSError *error){
			if(error == nil){
				NSMutableDictionary* tempCache = [NSMutableDictionary dictionaryWithCapacity:[scores count]];
				for (GKAchievement* score in scores){
					[tempCache setObject:score forKey:score.identifier];
				}
				self.earnedAchievementCache = tempCache;
				[self submitAchievement:identifier percentComplete:percentComplete withCompletionHandler:completionHandler];
			}
			else{
				//Something broke loading the achievement list.  Error out, and we'll try again the next time achievements submit.
				//TODO: cache this achievement until we're back online... i believe you'll need to do something like create the achievement
			}
            
            if (completionHandler) {
                //we don't have a cached achievement to hand back.. we could create one
                completionHandler(nil, error);
            }
		}];
	}
	else{
        //Search the list for the ID we're using...
		GKAchievement* achievement = [self.earnedAchievementCache objectForKey:identifier];
		if(achievement != nil){
			if((achievement.percentComplete >= 100.0) || (achievement.percentComplete >= percentComplete)){
				//Achievement has already been earned so we're done.
				achievement = nil;
			}
			achievement.percentComplete = percentComplete;
		}
		else{
			achievement = [[[GKAchievement alloc] initWithIdentifier: identifier] autorelease];
			achievement.percentComplete = percentComplete;
			//Add achievement to achievement cache...
			[self.earnedAchievementCache setObject: achievement forKey: achievement.identifier];
		}
        
		if(achievement != nil){
            //only shown when completion 100%
            
            //TODO: this class should be dumbed down and we make a new iOS5 class that compliments the iOS4 class
            //this may be one edge case that we have no choice but  to check in the base class
            if ([GameCenterManager areNewgameCenterFeaturesSupported]) {
                achievement.showsCompletionBanner = YES;
            }
            
            //TODO:possible to create your own custom banners
           // [GKNotificationBanner showBannerWithTitle:@"Title" message:@"Message" completionHandler:nil];
            
			//Submit the Achievement...
			[achievement reportAchievementWithCompletionHandler: ^(NSError *error){
                //TODO: cache this achievement until we're back online
                if (completionHandler) {
                    completionHandler(achievement, error);
                }
                
                if (error) {
#if TARGET_IPHONE_SIMULATOR
                    UIAlertView* alert = [[[UIAlertView alloc] initWithTitle: @"Error" message: [NSString stringWithFormat: @"submitAchievement: %@", [error localizedDescription]] delegate:self cancelButtonTitle: @"OK" otherButtonTitles:nil] autorelease];
                    [alert show];
#endif
                }
			}];
		}
	}
}

- (void)resetAchievements {
	[self resetAchievements:nil];
}

- (void)resetAchievements:(void(^)(NSError *error))completionHandler{
    self.earnedAchievementCache = nil;
	[GKAchievement resetAchievementsWithCompletionHandler: ^(NSError *error) {
        if (error) {
#if TARGET_IPHONE_SIMULATOR
            UIAlertView* alert = [[[UIAlertView alloc] initWithTitle: @"Error" message: [NSString stringWithFormat: @"resetAchievements: %@", [error localizedDescription]] delegate:self cancelButtonTitle: @"OK" otherButtonTitles:nil] autorelease];
            [alert show];
#endif
        }
        
        if (completionHandler) {
            completionHandler(error);
        }
	}];
}

#pragma mark -
#pragma Player Getters

- (void)mapPlayerIDtoPlayer:(NSString *)playerID withCompletionHandler:(void(^)(NSArray *playerArray, NSError *error))completionHandler{
	[GKPlayer loadPlayersForIdentifiers: [NSArray arrayWithObject: playerID] withCompletionHandler:^(NSArray *playerArray, NSError *error){
		GKPlayer* player = NULL;
		for (GKPlayer* tempPlayer in playerArray){
			if([tempPlayer.playerID isEqualToString: playerID]){
				player = tempPlayer;
				break;
			}
		}
		
        if (completionHandler) {
            completionHandler(playerArray, error);
        }
	}];
	
}

- (GKPlayer *)localPlayer{
    return [GKLocalPlayer localPlayer];
}

- (BOOL)gameCenterFeaturesEnabled{
    return [GKLocalPlayer localPlayer].authenticated;
}

//TODO: get other player's photos... this just does local
- (void)setPlayerPhotoWithUIImageView:(UIImageView *)imageView{
    [[GKLocalPlayer localPlayer] loadPhotoForSize:GKPhotoSizeNormal withCompletionHandler:^(UIImage *photo, NSError *error) {
        if (photo) {
            imageView.image = photo;
        }
        else{
            //set with default image;
        }
    }];
}

@end
