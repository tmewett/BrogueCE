//
//  UIViewController+UIViewController_GCLeaderBoardView.m
//  GameCenterTest
//
//  Created by Seth Howard on 8/16/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "UIViewController+UIViewController_GCLeaderBoardView.h"

@implementation UIViewController (UIViewController_GCLeaderBoardView)

#pragma mark -
#pragma LeaderBoards

//category can be nil
- (void)rgGCshowLeaderBoardWithCategory:(NSString *)category{
    GKLeaderboardViewController *leaderBoardController = [[[GKLeaderboardViewController alloc] init] autorelease];
    
    leaderBoardController.category = category;
    leaderBoardController.timeScope = GKLeaderboardTimeScopeWeek;
    leaderBoardController.leaderboardDelegate = self;
    
    [self presentModalViewController:leaderBoardController animated:YES];
}

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController{
    [self dismissModalViewControllerAnimated:YES];
}

@end
