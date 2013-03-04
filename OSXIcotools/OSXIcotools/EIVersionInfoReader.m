//
//  EIVersionInfoReader.m
//  EXEIconReader
//
//  Created by Daniele Cattaneo on 18/11/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

//  Thanks to Raymond Chen (MSFT) for the useful insight on the VERSIONINFO resource format.
//  http://blogs.msdn.com/b/oldnewthing/archive/2006/12/22/1348663.aspx (kludge)
//  http://blogs.msdn.com/b/oldnewthing/archive/2006/12/21/1340571.aspx (32 bit)
//  http://blogs.msdn.com/b/oldnewthing/archive/2006/12/20/1332035.aspx (16 bit)

#import "EIVersionInfoReader.h"

typedef struct {
  int16_t cbNode;  //Size of the node (a node includes its children)
  int16_t cbData;  //Size of the data in the node
  int16_t wType;   //1 if the node contains a string
} VERSIONNODE_HEADER;

typedef struct {
  int16_t cbNode;  //Size of the node (a node includes its children)
  int16_t cbData;  //Size of the data in the node
} VERSIONNODE16_HEADER;


NSStringEncoding NSEncodingFromCodePage(int cp) {
  switch (cp) {
    case 20127: return NSASCIIStringEncoding;
    case 20932: return NSJapaneseEUCStringEncoding;
    case 65001: return NSUTF8StringEncoding;
    case 28591: return NSISOLatin1StringEncoding;
    case   932: return NSShiftJISStringEncoding;
    case 28592: return NSISOLatin2StringEncoding;
    case  1251: return NSWindowsCP1251StringEncoding;    /* Cyrillic; same as AdobeStandardCyrillic */
    case  1252: return NSWindowsCP1252StringEncoding;    /* WinLatin1 */
    case  1253: return NSWindowsCP1253StringEncoding;    /* Greek */
    case  1254: return NSWindowsCP1254StringEncoding;    /* Turkish */
    case  1250: return NSWindowsCP1250StringEncoding;    /* WinLatin2 */
    case 50222:
    case 50221:
    case 50220: return NSISO2022JPStringEncoding;        /* ISO 2022 Japanese encoding for e-mail */
    case 10000: return NSMacOSRomanStringEncoding;
    case  1201: return NSUTF16BigEndianStringEncoding;   /* NSUTF16StringEncoding encoding with explicit endianness specified */
    case  1200: return NSUTF16LittleEndianStringEncoding;/* NSUTF16StringEncoding encoding with explicit endianness specified */                
    case 12001: return NSUTF32BigEndianStringEncoding;   /* NSUTF32StringEncoding encoding with explicit endianness specified */
    case 12000: return NSUTF32LittleEndianStringEncoding;/* NSUTF32StringEncoding encoding with explicit endianness specified */
    default:    return NSUTF16LittleEndianStringEncoding;/* We hope exotic codings are just misguided mistakes.*/
  }
}


int utf16StringLen(int16_t* string) {
  int len = 0;
  while (len < 1024) {
    if (*string == 0) return len;
    len += 1;
    string += 1;
  }
  return -1;  //absurd string length
}

NSArray* makeRequest(NSString* subBlock) {
  NSArray *request;
  if ([subBlock compare:@"\\"] == NSOrderedSame) {
    request = [NSArray arrayWithObject:@"VS_VERSION_INFO"];
  } else {
    request = [[NSString stringWithFormat:@"VS_VERSION_INFO%@",subBlock] componentsSeparatedByString:@"\\"];
  }
  return request;
}


#define PAD_TO_32BIT(x) (((void*)((x)) + ((4 - ((intptr_t)((x)) % 4)) % 4)))
#define ERR_RET(x) {*err = ((x)); return nil;}

NSData* resTreeRead32(NSArray* path, int level, NSData* block, EIVERSION_ERR* err, BOOL returnWholeBlock) {
  const void* bb = [block bytes];
  const void* be = bb + [block length];
  const VERSIONNODE_HEADER* vnh = bb;
  *err = EIV_NOERR;
  
  const void* wszName = vnh + 1;
  while ((void*)vnh < be) {
    //  The following line uncovers a bug in the operating system.
    //  Uglier replacement follows.
    //NSString* temp = [NSString stringWithCString:(const char*)wszName encoding:NSUTF16StringEncoding];
    int len = utf16StringLen((int16_t*)wszName);
    if (len == -1) ERR_RET(EIV_WRONGFORMAT);
    NSString *temp = [NSString stringWithCharacters:(const unichar*)wszName length:len];
    
    if (([temp caseInsensitiveCompare:[path objectAtIndex:level]]==NSOrderedSame) || 
        ([@"*" compare:[path objectAtIndex:level]]==NSOrderedSame)) {  //* is a wildcard for the first item
      const void* dataptr = PAD_TO_32BIT(wszName + (len * 2) + 2);
      const void* nodeptr = PAD_TO_32BIT(dataptr + vnh->cbData);  //pointer to first child node
      if ([path count] == (level+1)) {
        if (!returnWholeBlock) {
          intptr_t realDataLen = vnh->cbData;
          if ((vnh->wType == 1) && (level == 3)) {
            if ((nodeptr - (void*)vnh) < (vnh->cbNode)) realDataLen *= 2;
            //Right after the transition to Unicode version strings many resource compilers had a bug where
            //the cbData is interpreted as a character count, not as a byte count as it should be.
          }
          return [NSData dataWithBytesNoCopy:(void*)dataptr length:realDataLen freeWhenDone:NO];
        } else {
          return [NSData dataWithBytesNoCopy:(void*)nodeptr length:(((void*)vnh+(vnh->cbNode))-nodeptr) freeWhenDone:NO];
        }
      } else {
        NSData *nodeData = [NSData dataWithBytesNoCopy:(void*)nodeptr length:(((void*)vnh+(vnh->cbNode))-nodeptr) freeWhenDone:NO];
        return resTreeRead32(path, level+1, nodeData, err, returnWholeBlock);
      }
    }
    
    vnh = PAD_TO_32BIT((void*)vnh + vnh->cbNode);
    wszName = vnh + 1;
  }
  ERR_RET(EIV_UNKNOWNNODE);
}

NSData* resTreeRead16(NSArray* path, int level, NSData* block, EIVERSION_ERR* err, BOOL returnWholeBlock) {
  const void* bb = [block bytes];
  const void* be = bb + [block length];
  const VERSIONNODE16_HEADER* vnh = bb;
  *err = EIV_NOERR;
  
  const void* wszName = vnh + 1;
  while ((void*)vnh < be) {
    NSString* temp = [NSString stringWithCString:(const char*)wszName encoding:NSWindowsCP1252StringEncoding];
        
    if (([temp caseInsensitiveCompare:[path objectAtIndex:level]]==NSOrderedSame) || 
        ([@"*" compare:[path objectAtIndex:level]]==NSOrderedSame)) {  //* is a wildcard for the first item
      const void* dataptr = PAD_TO_32BIT(wszName + [temp length] + 1);
      const void* nodeptr = PAD_TO_32BIT(dataptr + vnh->cbData);  //pointer to first child node
      if ([path count] == (level+1)) {
        if (!returnWholeBlock) {
          return [NSData dataWithBytesNoCopy:(void*)dataptr length:vnh->cbData freeWhenDone:NO];
        } else {
          return [NSData dataWithBytesNoCopy:(void*)nodeptr length:(((void*)vnh+(vnh->cbNode))-nodeptr) freeWhenDone:NO];
        }
      } else {
        NSData *nodeData = [NSData dataWithBytesNoCopy:(void*)nodeptr length:(((void*)vnh+(vnh->cbNode))-nodeptr) freeWhenDone:NO];
        return resTreeRead16(path, level+1, nodeData, err, returnWholeBlock);
      }
    }
    
    vnh = PAD_TO_32BIT((void*)vnh + vnh->cbNode);
    wszName = vnh + 1;
  }
  ERR_RET(EIV_UNKNOWNNODE);
}



@implementation EIVersionInfoReader

- initWithBlock:(NSData*)myBlock is16Bit:(BOOL)win16 {
  gBlock = [myBlock retain];
  win16Block = win16;
  return [super init];
}

- (void)dealloc {
  [gBlock release];
  [super dealloc];
}

//Like VerQueryValue(pBlock, lpSubBlock, lplpBuffer, puLen);
- (NSData*)queryValue:(NSString*)subBlock error:(EIVERSION_ERR*)err {
  if (win16Block) {
    return resTreeRead16(makeRequest(subBlock), 0, gBlock, err, NO);
  } else {
    return resTreeRead32(makeRequest(subBlock), 0, gBlock, err, NO);
  }
}

- (NSArray*)querySubNodesUnder:(NSString*)subBlock error:(EIVERSION_ERR*)err {
  NSData *queryNode;
  if (win16Block) {
    queryNode = resTreeRead16(makeRequest(subBlock), 0, gBlock, err, YES);
  } else {
    queryNode = resTreeRead32(makeRequest(subBlock), 0, gBlock, err, YES);
  }
  if (*err) return nil;
  
  NSMutableArray* nodeArray = [[[NSMutableArray alloc] init] autorelease];
  
  const void* bb = [queryNode bytes];
  const void* be = bb + [queryNode length];
  const VERSIONNODE_HEADER* vnh = bb;
  
  const void* wszName = vnh + 1;
  if (win16Block) wszName -= 2;
  while ((void*)vnh < be) {
    NSString *temp;
    if (win16Block) {
      temp = [NSString stringWithCString:(const char*)wszName encoding:NSWindowsCP1252StringEncoding];
    } else {
      int len = utf16StringLen((int16_t*)wszName);
      if (len == -1) ERR_RET(EIV_WRONGFORMAT);
      temp = [NSString stringWithCharacters:(const unichar*)wszName length:len];
    }
    [nodeArray addObject:temp];
    
    vnh = PAD_TO_32BIT((void*)vnh + vnh->cbNode);
    wszName = vnh + 1;
    if (win16Block) wszName -= 2;
  }
  return nodeArray;
}

- (TRANSLATION)bestLocaleWithError:(EIVERSION_ERR*)err {
  TRANSLATION retv;
  NSData* transl = [self queryValue:@"\\VarFileInfo\\Translation" error:err];
  if (*err) return retv;
  
  if (([transl length] % sizeof(TRANSLATION)) != 0) {
    *err = EIV_WRONGDATA;
    return retv;
  }
  int langCount = [transl length] / sizeof(TRANSLATION);
  
  TRANSLATION* availLangs = (TRANSLATION*)[transl bytes];
  NSArray* sysLangs = [NSLocale preferredLanguages];
  
  int bestIndex = 0; 
  NSUInteger bestLevel = -1;
  for (int c=0; c<langCount; c++) {
    //The reason why Apple engineers added this function in 10.6 is beyond me...
    //...but it's great!
    NSString* localeId = [NSLocale localeIdentifierFromWindowsLocaleCode:(availLangs[c].lang)];
    NSUInteger thisLevel = [sysLangs indexOfObject:localeId];
    if ((thisLevel != NSNotFound) && (thisLevel > bestLevel)) {
      bestLevel = thisLevel;
      bestIndex = c;
    }
  }
  
  //return [NSString stringWithFormat:@"%04X%04X", availLangs[bestIndex].lang, availLangs[bestIndex].coding];
  retv.lang = availLangs[bestIndex].lang;
  retv.coding = availLangs[bestIndex].coding;
  return retv;
}



@end
