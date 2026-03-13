//
//  RogueDriver.m
//  Brogue
//
//  Created by Brian and Kevin Walker on 12/26/08.
//  Updated for iOS by Seth Howard on 03/01/13
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

#include <limits.h>
#include <unistd.h>
#include "CoreFoundation/CoreFoundation.h"
#import "RogueDriver.h"
#include "Rogue.h"
//#import "GameCenterManager.h"
#import <QuartzCore/QuartzCore.h>

extern "C" {
    #include "GlobalsBase.h"
    #include "platform.h"

    extern CBrogueGameEvent uiMode;
    extern char uiTextEntry[BROGUE_FILENAME_MAX];
}



#define kRateScore 3000

#define BROGUE_VERSION	4	// A special version number that's incremented only when
// something about the OS X high scores file structure changes.

boolean hasGraphics = true;
boolean serverMode = false;
boolean keyboardPresent = false;            // no keyboard until key pressed, set in nextKeyOrMouseEvent()
enum graphicsModes graphicsMode = TEXT_GRAPHICS; // start in TEXT_GRAPHICS till mode switched


// Objective-c Bridge

static CGColorSpaceRef _colorSpace;
// quick and easy bridge for C/C++ code. Could be cleaned up.
static SKViewPort *skviewPort;
static BrogueViewController *brogueViewController;

@implementation RogueDriver 

+ (id)sharedInstanceWithViewPort:(SKViewPort *)viewPort viewController:(BrogueViewController *)viewController {
    static RogueDriver *instance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[RogueDriver alloc] init];
        brogueViewController = viewController;
        skviewPort = viewPort;
    });
    
    return instance;
}

- (id)init
{
    self = [super init];
    if (self) {
        if (!_colorSpace) {
            _colorSpace = CGColorSpaceCreateDeviceRGB();
        }
    }
    return self;
}

+ (unsigned long)rogueSeed {
    return rogue.seed;
}

@end

//  plotChar: plots inputChar at (xLoc, yLoc) with specified background and foreground colors.
//  Color components are given in ints from 0 to 100.

void _plotChar(enum displayGlyph inputChar,
			  short xLoc, short yLoc,
			  short foreRed, short foreGreen, short foreBlue,
			  short backRed, short backGreen, short backBlue) {
    unsigned int glyphCode;
    
   // NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    CGFloat backComponents[] = {(CGFloat)(backRed * .01), (CGFloat)(backGreen * .01), (CGFloat)(backBlue * .01), 1.};
    CGColorRef backColor = CGColorCreate(_colorSpace, backComponents);

    CGFloat foreComponents[] = {(CGFloat)(foreRed * .01), (CGFloat)(foreGreen * .01), (CGFloat)(foreBlue * .01), 1.};
    CGColorRef foreColor = CGColorCreate(_colorSpace, foreComponents);

    
    if ( (inputChar > G_DOWN_ARROW) &&
         ((graphicsMode == TILES_GRAPHICS) ||
         ((graphicsMode == HYBRID_GRAPHICS) && (isEnvironmentGlyph(inputChar)))) ) {
        glyphCode = (inputChar-130) + 0x4000;
    } else {
        glyphCode = glyphToUnicode(inputChar);
    }
    
    [skviewPort setCellWithX:xLoc y:yLoc code:glyphCode bgColor:backColor fgColor:foreColor];
    
    CGColorRelease(backColor);
    CGColorRelease(foreColor);
}

__unused void pausingTimerStartsNow(void) {}

// Returns true if the player interrupted the wait with a keystroke; otherwise false.
boolean _pauseForMilliseconds(short milliseconds, PauseBehavior behavior) {
    BOOL hasEvent = NO;

    // brturn: Update the UI Mode every frame
    if (brogueViewController.lastBrogueGameEvent != (BrogueGameEvent)uiMode) {
        brogueViewController.brogueTextInput = [NSString stringWithUTF8String:uiTextEntry];
        brogueViewController.lastBrogueGameEvent = (BrogueGameEvent)uiMode;
    }
    
    [NSThread sleepForTimeInterval:milliseconds/1000.];

    if (brogueViewController.hasTouchEvent || brogueViewController.hasKeyEvent) {
        hasEvent = YES;
    }

	return hasEvent;
}

void _nextKeyOrMouseEvent(rogueEvent *returnEvent, __unused boolean textInput, boolean colorsDance) {
	short x, y;
    float width = [[UIScreen mainScreen] bounds].size.width;
    float height = [UIScreen safeBounds].size.height;
    for(;;) {
        // we should be ok to block here. We don't seem to call pauseForMilli and this at the same time
        // 60Hz
        [NSThread sleepForTimeInterval:0.016667];
        
        // brturn: Update the UI Mode every frame
        if (brogueViewController.lastBrogueGameEvent != (BrogueGameEvent)uiMode) {
            brogueViewController.brogueTextInput = [NSString stringWithUTF8String:uiTextEntry];
            brogueViewController.lastBrogueGameEvent = (BrogueGameEvent)uiMode;
        }
        
        if (colorsDance) {
            shuffleTerrainColors(3, true);
            commitDraws();
        }
        
        if ([brogueViewController hasKeyEvent]) {
            returnEvent->eventType = KEYSTROKE;
            returnEvent->param1 = [brogueViewController dequeKeyEvent];
            //printf("\nKey pressed: %i", returnEvent->param1);
            returnEvent->param2 = 0;
            returnEvent->controlKey = 0;//([theEvent modifierFlags] & NSControlKeyMask ? 1 : 0);
            returnEvent->shiftKey = 0;//([theEvent modifierFlags] & NSShiftKeyMask ? 1 : 0);
            keyboardPresent = brogueViewController.keyboardDetected; // set a global if we've had a key pressed on a physical keyboard.
            break;
        }
        if (brogueViewController.hasTouchEvent) {
            UIBrogueTouchEvent *touch = [brogueViewController dequeTouchEvent];
            
            if (touch.phase != UITouchPhaseCancelled) {
                switch (touch.phase) {
                    case UITouchPhaseBegan:
                    case UITouchPhaseStationary:
                        returnEvent->eventType = MOUSE_DOWN;
                        break;
                    case UITouchPhaseEnded:
                        returnEvent->eventType = MOUSE_UP;
                        break;
                    case UITouchPhaseMoved:
                        returnEvent->eventType = MOUSE_ENTERED_CELL;
                        break;
                    default:
                        break;
                }
                
                x = COLS * float(touch.location.x) / width;
                y = ROWS * float(touch.location.y) / height;
                
                returnEvent->param1 = x;
                returnEvent->param2 = y;
                returnEvent->controlKey = 0;
                returnEvent->shiftKey = 0;
                
                break;
            }
        }
    }
}

#pragma mark - bridge

boolean _controlKeyIsDown(void) {
    return brogueViewController.controlKeyDown;
}

boolean _shiftKeyIsDown(void) {
    return brogueViewController.shiftKeyDown;
}

//void submitAchievementForCharString(char *achievementKey) {
//    [[GameCenterManager sharedInstance] submitAchievement:[NSString stringWithUTF8String:achievementKey] percentComplete:100.];
//}


void initializeLaunchArguments(enum NGCommands *command, char *path, unsigned long *seed) {
	//*command = NG_SCUM;
    *command = NG_NOTHING;
	path[0] = '\0';
	*seed = 0;
}

void initializeBrogueSaveLocation(void) {
      
    NSFileManager *manager = [NSFileManager defaultManager];
    NSError *err;
    
    // Look up the full path to the user's Application Support folder (usually ~/Library/Application Support/).
    NSString *basePath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex: 0];
    
    // Use a folder under Application Support named after the application.
  //  NSString *appName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleName"];
    NSString *documentsPath = basePath;//[basePath stringByAppendingPathComponent: appName];
    
    // Create our folder the first time it is needed.
    if (![manager fileExistsAtPath:documentsPath]) {
        [manager createDirectoryAtPath:documentsPath withIntermediateDirectories:YES attributes:nil error:&err];
    }
    
    // Set the working directory to this path, so that savegames and recordings will be stored here.
    [manager changeCurrentDirectoryPath:documentsPath];
}

extern struct brogueConsole iosConsole;

void brogueMainIOS() {
    currentConsole = iosConsole;
    initializeBrogueSaveLocation();
    rogueMain();
}

boolean _modifierHeld(int modifier) {
    return _controlKeyIsDown() || _shiftKeyIsDown();
}

enum graphicsModes _setGraphicsMode(enum graphicsModes newMode) {
    // for now, just cycle through the choices, but don't do anything
    return newMode;

}

struct brogueConsole iosConsole = {
    brogueMainIOS,          // initialize data structure, call rogueMain
    _pauseForMilliseconds,  // pause, return boolean if input event available
    _nextKeyOrMouseEvent,   // block until event available
    _plotChar,              // draw a character at a location, with colors
    NULL,                   // remap keyboard keys
    _modifierHeld,          // is modifier held? flags, 0 for shift, 1 for Ctrl
    
    // optional
    NULL,                   // *notifyEvent : call-back for certain events
    NULL,                   // *takeScreenshot
    _setGraphicsMode,       // set graphics mode: TEXT, TILE, HYBRID
};
