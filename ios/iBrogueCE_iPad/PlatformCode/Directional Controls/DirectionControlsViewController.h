//
//  DirectionControlsViewController.h
//  iBrogue_iPad
//
//  Created by Seth Howard on 7/12/14.
//  Copyright (c) 2014 Seth howard. All rights reserved.
//

#import <UIKit/UIKit.h>

// TODO: rewrite in swift so it's not so shitty

typedef NS_ENUM(NSInteger, ControlDirection) {
    ControlDirectionCatchAll = 0,
    ControlDirectionUp,
    ControlDirectionRight,
    ControlDirectionDown,
    ControlDirectionLeft,
    ControlDirectionUpLeft,
    ControlDirectionUpRight,
    ControlDirectionDownRight,
    ControlDirectionDownLeft,
};

extern NSString * _Nonnull kUP_Key;
extern NSString * _Nonnull kRIGHT_key;
extern NSString * _Nonnull kDOWN_key;
extern NSString * _Nonnull kLEFT_key;
extern NSString * _Nonnull kUPLEFT_key;
extern NSString * _Nonnull kUPRight_key;
extern NSString * _Nonnull kDOWNLEFT_key;
extern NSString * _Nonnull kDOWNRIGHT_key;

@interface DirectionControlsViewController : UIViewController

- (void)cancel;

@property (nonatomic, strong, nullable) UIButton *directionalButton;
@property (nonatomic, readonly) BOOL areDirectionalControlsHidden;

@end
