//
//  UIViewController+UIViewController_GCAchievementView.m
//  GameCenterTest
//
//  Created by Seth Howard on 8/16/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "UIViewController+UIViewController_GCAchievementView.h"

@implementation UIViewController (UIViewController_GCAchievementView)

#pragma mark -
#pragma Achievements

- (void)rgGCshowAchievements{
    GKAchievementViewController *achievementsViewController = [[[GKAchievementViewController alloc] init] autorelease];
    achievementsViewController.achievementDelegate = self;
    [self presentModalViewController:achievementsViewController animated:YES];
}

- (void)achievementViewControllerDidFinish:(GKAchievementViewController *)viewController{
    [self dismissModalViewControllerAnimated:YES];
}

@end
