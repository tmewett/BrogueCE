//
//  DirectionControlsViewController.m
//  iBrogue_iPad
//
//  Created by Seth Howard on 7/12/14.
//  Copyright (c) 2014 Seth howard. All rights reserved.
//

#import "DirectionControlsViewController.h"

NSString *kUP_Key = @"k";
NSString *kRIGHT_key = @"l";
NSString *kDOWN_key = @"j";
NSString *kLEFT_key = @"h";
NSString *kUPLEFT_key = @"y";
NSString *kUPRight_key = @"u";
NSString *kDOWNLEFT_key = @"b";
NSString *kDOWNRIGHT_key = @"n";

@interface DirectionControlsViewController ()
// we use the container to animate hide and show while keeping the parent view on the parent layer
@property (weak, nonatomic) IBOutlet UIView *controlsContainer;
@property (nonatomic, assign, getter = isButtonDown) BOOL buttonDown;

@property (weak, nonatomic) IBOutlet UIView *dragAreaView;
@property (nonatomic, strong) NSTimer *repeatTimer;
@end

@implementation DirectionControlsViewController

#pragma mark -

- (void)viewDidLoad {
    [super viewDidLoad];
    [self showDraggableArea];
}

- (void)showDraggableArea {
    [UIView animateWithDuration:0.4 animations:^{
        self.dragAreaView.alpha = 0.4;
    }];
}

- (void)hideDraggableArea {
    if (self.dragAreaView.alpha > 0) {
        [UIView animateWithDuration:0.4 animations:^{
            self.dragAreaView.alpha = 0;
        }];
    }
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [super touchesEnded:touches withEvent:event];
    [self hideDraggableArea];
}

- (void)handleRepeatKeyPress {
    // trigger the KVO
    self.directionalButton = self.directionalButton;
}

- (void)cancel {
    [self buttonUp:self];
    [self hideDraggableArea];
}

- (IBAction)buttonDown:(id)sender {
    self.directionalButton = (UIButton *)sender;
    self.buttonDown = YES;
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.4 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        if (self.buttonDown) {
            [self.repeatTimer invalidate];
            self.repeatTimer = nil;
            self.repeatTimer = [NSTimer scheduledTimerWithTimeInterval:0.08 target:self selector:@selector(handleRepeatKeyPress) userInfo:nil repeats:YES];
        }
    });
    
    [self hideDraggableArea];
}

- (IBAction)buttonUp:(id)sender {
    self.buttonDown = NO;
    [self.repeatTimer invalidate];
    self.repeatTimer = nil;
    self.directionalButton = nil;
}

@end
