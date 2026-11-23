//
//  AppDelegate.m
//  iBrogue_iPad
//
//  Created by Seth Howard on 2/22/13.
//  Copyright (c) 2013 Seth howard. All rights reserved.
//

#import "AppDelegate.h"

const int kROWS = ROWS;
const int kCOLS = COLS;
const int kSTAT_BAR_WIDTH = STAT_BAR_WIDTH;
const int kMESSAGE_LINES = MESSAGE_LINES;
const int kPROWS = ROWS - MESSAGE_LINES + 1;

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    application.applicationSupportsShakeToEdit = YES;
    return YES;
}

@end
