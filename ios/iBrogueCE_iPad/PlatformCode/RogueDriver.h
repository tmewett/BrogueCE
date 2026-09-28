//
//  RogueMain.h
//  Brogue
//
//  Created by Brian and Kevin Walker on 12/26/08.
//  Updated for iOS by Seth Howard on 03/01/13.
//  Copyright 2012. All rights reserved.
//  
//  This file is part of Brogue.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// Used as a bridge between the c and the objective-c

#import <UIKit/UIKit.h>

typedef NS_ENUM(NSInteger, BrogueGameEvent) {
    BrogueGameEventInMenu = 0,
    BrogueGameEventInNormalPlay,
    BrogueGameEventShowEscape,
    BrogueGameEventShowKeyboardAndEscape,
};


@class SKViewPort;
@interface RogueDriver : NSObject

+ (id)sharedInstanceWithViewPort:(SKViewPort *)viewPort viewController:(UIViewController *)viewController;
+ (unsigned long)rogueSeed;

@end
