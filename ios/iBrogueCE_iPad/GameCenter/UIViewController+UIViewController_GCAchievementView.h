//
//  UIViewController+UIViewController_GCAchievementView.h
//  GameCenterTest
//
//  Created by Seth Howard on 8/16/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GameCenterManager.h"

@interface UIViewController (UIViewController_GCAchievementView)<GKAchievementViewControllerDelegate>
- (void)rgGCshowAchievements;
@end
