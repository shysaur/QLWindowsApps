//
//  EIVersionInfoReader.h
//  EXEIconReader
//
//  Created by Daniele Cattaneo on 18/11/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
  EIV_NOERR = 0,
  EIV_UNKNOWNNODE = 1,
  EIV_WRONGFORMAT = 2,
  EIV_WRONGDATA = 3
} EIVERSION_ERR;

typedef struct {
  uint16_t lang;
  uint16_t coding;
} TRANSLATION;

NSStringEncoding NSEncodingFromCodePage(int cp);

@interface EIVersionInfoReader : NSObject {
  NSData* gBlock;
  BOOL win16Block;
}

- initWithBlock:(NSData*)myBlock is16Bit:(BOOL)win16;
- (void)dealloc;

- (NSData*)queryValue:(NSString*)subBlock error:(EIVERSION_ERR*)err;
- (NSArray*)querySubNodesUnder:(NSString*)subBlock error:(EIVERSION_ERR*)err;

//Note: "getBestLocaleWithError:" is *bugged* because it does a string comparison on locales, which
//is often incorrect. I think the effort to write code to handle multiple locales in one resource is quite
//wasted because it never happens; developers who want different version informations by language
//use the resource tree locale node, not multiple locales in one resource.
- (TRANSLATION)bestLocaleWithError:(EIVERSION_ERR*)err;

@end


