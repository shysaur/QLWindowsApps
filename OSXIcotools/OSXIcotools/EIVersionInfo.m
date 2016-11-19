/* EIVersionInfoReader.m - Class used to interpret VERSIONINFO structures
 *
 * Copyright (C) 2012-13 Daniele Cattaneo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*  Thanks to Raymond Chen (MSFT) for the useful insight on the VERSIONINFO resource format.
 *  http://blogs.msdn.com/b/oldnewthing/archive/2006/12/22/1348663.aspx (kludge)
 *  http://blogs.msdn.com/b/oldnewthing/archive/2006/12/21/1340571.aspx (32 bit)
 *  http://blogs.msdn.com/b/oldnewthing/archive/2006/12/20/1332035.aspx (16 bit)
 */

#import "EIVersionInfo.h"


#define PAD_TO_32BIT(x) (((void*)((x)) + ((4 - ((intptr_t)((x)) % 4)) % 4)))
#define ERR_RET(x) {*err = ((x)); return nil;}
#define CHECK_PTR(b, x, e) {if ((void*)(x) < b || e < (void*)(x)) ERR_RET(EIV_WRONGFORMAT);}


typedef struct {
  int16_t cbNode;  //Size of the node (a node includes its children)
  int16_t cbData;  //Size of the data in the node
  int16_t wType;   //1 if the node contains a string
} VERSIONNODE_HEADER;

typedef struct {
  int16_t cbNode;  //Size of the node (a node includes its children)
  int16_t cbData;  //Size of the data in the node
} VERSIONNODE16_HEADER;


int EIUTF16CheckedStringLen(const unichar* string, const unichar *maxptr, BOOL expectTerm) {
  int len;
  
  for (len = 0; string < maxptr; len++)
    if (*(string++) == '\0')
      return len;
  return expectTerm ? -1 : len - 1;
}


int EICheckedStringLen(const char *str, const char *maxptr, BOOL expectTerm) {
  int len;
  
  for (len = 0; str < maxptr; len++)
    if (*(str++) == '\0')
      return len;
  return expectTerm ? -1 : len - 1;
}


NSArray *EIRequestFromString(NSString *subBlock) {
  NSArray *request;
  NSString *tmp;
  
  if ([subBlock isEqual:@"\\"]) {
    request = [NSArray arrayWithObject:@"VS_VERSION_INFO"];
  } else {
    tmp = [@"VS_VERSION_INFO" stringByAppendingString:subBlock];
    request = [tmp componentsSeparatedByString:@"\\"];
  }
  return request;
}


NSData *EIResTreeRead32(NSArray *path, int level, NSData *block, EIVERSION_ERR *err, BOOL getChildren) {
  BOOL found;
  const void *bb, *be;
  const VERSIONNODE_HEADER *vnh;
  const unichar *wszName;
  ssize_t nameLen, subtreeLen;
  const void *dataptr, *nextptr;
  NSString *nodeName, *nodeToRead;
  NSData *nodeData;
  
  vnh = bb = [block bytes];
  be = bb + [block length];
  *err = EIV_NOERR;
  
  wszName = (unichar *)(vnh + 1);
  nodeToRead = [path objectAtIndex:level];
  
  /* Search for the right node in this level */
  found = NO;
  while ((void*)vnh < be) {
    nameLen = EIUTF16CheckedStringLen(wszName, be, YES);
    if (nameLen < 0) ERR_RET(EIV_WRONGFORMAT);
    
    nodeName = [NSString stringWithCharacters:wszName length:nameLen];
    if ([nodeName caseInsensitiveCompare:nodeToRead] == NSOrderedSame || [@"*" isEqual:nodeToRead]) {
      found = YES;
      break;
    }
    
    vnh = PAD_TO_32BIT((void*)vnh + vnh->cbNode);
    wszName = (const unichar*)(vnh + 1);
  }
  if (!found)
    ERR_RET(EIV_UNKNOWNNODE);
  
  dataptr = PAD_TO_32BIT((void*)(wszName + nameLen + 1));
  nextptr = PAD_TO_32BIT(dataptr + vnh->cbData);  //pointer to first child node
  subtreeLen = ((void*)vnh + vnh->cbNode) - nextptr;
  
  CHECK_PTR(bb, nextptr + subtreeLen - 1, be);
  
  /* If we're at the node we want to return, return it */
  if ([path count] == level + 1) {
    if (getChildren) {
      return [NSData dataWithBytes:nextptr length:subtreeLen];
    } else {
      /* Right after the transition to Unicode version strings many resource
       * compilers had a bug where the cbData is interpreted as a character
       * count, not as a byte count as it should be.*/
      if (vnh->wType == 1 && level == 3)
        if (nextptr - (void*)vnh < vnh->cbNode)
          return [NSData dataWithBytes:dataptr length:vnh->cbData * 2];
      return [NSData dataWithBytes:dataptr length:vnh->cbData];
    }
  }
  
  /* Otherwise recurse into this node's children */
  nodeData = [NSData dataWithBytesNoCopy:(void*)nextptr length:subtreeLen freeWhenDone:NO];
  return EIResTreeRead32(path, level+1, nodeData, err, getChildren);
}


NSData *EIResTreeRead16(NSArray *path, int level, NSData* block, EIVERSION_ERR *err, BOOL getChildren) {
  BOOL found;
  const void *bb, *be;
  const VERSIONNODE16_HEADER *vnh;
  const char *wszName;
  ssize_t subtreeLen, sl;
  const void *dataptr, *nextptr;
  NSString *nodeName, *nodeToRead;
  NSData *nodeData;
  
  vnh = bb = [block bytes];
  be = bb + [block length];
  *err = EIV_NOERR;
  
  wszName = (char*)(vnh + 1);
  nodeToRead = [path objectAtIndex:level];
  
  /* Search for the right node in this level */
  found = NO;
  while ((void*)vnh < be) {
    sl = EICheckedStringLen(wszName, be, YES);
    if (sl < 0) ERR_RET(EIV_WRONGFORMAT);
    nodeName = [NSString stringWithCString:wszName encoding:NSWindowsCP1252StringEncoding];
        
    if ([nodeName caseInsensitiveCompare:nodeToRead] == NSOrderedSame ||
        [@"*" isEqual:nodeToRead]) {
      found = YES;
      break;
    }
    
    vnh = PAD_TO_32BIT((void*)vnh + vnh->cbNode);
    wszName = (const char*)(vnh + 1);
  }
  
  if (!found)
    ERR_RET(EIV_UNKNOWNNODE);
  
  dataptr = PAD_TO_32BIT(wszName + [nodeName length] + 1);
  nextptr = PAD_TO_32BIT(dataptr + vnh->cbData);  //pointer to first child node
  subtreeLen = ((void*)vnh + vnh->cbNode) - nextptr;
  
  CHECK_PTR(bb, nextptr + subtreeLen - 1, be);
  
  /* If we're at the node we want to return, return it */
  if ([path count] == level + 1) {
    if (getChildren)
      return [NSData dataWithBytes:nextptr length:subtreeLen];
    return [NSData dataWithBytes:dataptr length:vnh->cbData];
  }
  
  /* Otherwise recurse into this node's children */
  nodeData = [NSData dataWithBytesNoCopy:(void*)nextptr length:subtreeLen freeWhenDone:NO];
  return EIResTreeRead16(path, level+1, nodeData, err, getChildren);
}


@implementation EIVersionInfo


- (instancetype)initWithData:(NSData *)myBlock is16Bit:(BOOL)win16 {
  gBlock = myBlock;
  win16Block = win16;
  return [super init];
}


- (BOOL)is16bit {
  return win16Block;
}


- (NSData *)data {
  return gBlock;
}


- (NSString *)queryStringValue:(NSString *)subBlock error:(EIVERSION_ERR *)err {
  NSData *raw;
  NSStringEncoding resEnc;
  uint8_t *bytes;
  int l;
  
  raw = [self queryValue:subBlock error:err];
  if (!raw)
    return nil;
  bytes = [raw bytes];
  
  if (win16Block) {
    l = EICheckedStringLen(bytes, bytes + [raw length], NO);
    return [[NSString alloc] initWithBytes:bytes length:l
            encoding:NSWindowsCP1252StringEncoding];
  }
  
  l = EIUTF16CheckedStringLen(bytes, bytes + [raw length], NO);
  return [NSString stringWithCharacters:bytes length:l];
}


// Like VerQueryValue(pBlock, lpSubBlock, lplpBuffer, puLen);
- (NSData *)queryValue:(NSString *)subBlock error:(EIVERSION_ERR *)err {
  NSData *res;
  EIVERSION_ERR rerr = EIV_NOERR;
  
  if (win16Block)
    res = EIResTreeRead16(EIRequestFromString(subBlock), 0, gBlock, &rerr, NO);
  else
    res = EIResTreeRead32(EIRequestFromString(subBlock), 0, gBlock, &rerr, NO);
  if (!res && err)
    *err = rerr;
  return res;
}


- (NSArray *)querySubNodesUnder:(NSString *)subBlock error:(EIVERSION_ERR *)err {
  NSArray *res;
  EIVERSION_ERR rerr = EIV_NOERR;
  
  if (win16Block)
    res = [self _query16BitSubNodesUnder:subBlock error:&rerr];
  else
    res = [self _query32BitSubNodesUnder:subBlock error:&rerr];
  if (!res && err)
    *err = rerr;
  return res;
}


- (NSArray *)_query16BitSubNodesUnder:(NSString *)subBlock error:(EIVERSION_ERR *)err {
  NSData *queryNode;
  NSMutableArray *nodeArray;
  const void *bb, *be;
  const VERSIONNODE16_HEADER *vnh;
  const char *wszName;
  NSString *nodeName;
  int sl;
  
  queryNode = EIResTreeRead16(EIRequestFromString(subBlock), 0, gBlock, err, YES);
  if (*err) return nil;
  
  nodeArray = [NSMutableArray array];
  
  vnh = bb = [queryNode bytes];
  be = bb + [queryNode length];
  
  wszName = (char*)(vnh + 1);
  while ((void*)vnh < be) {
    sl = EICheckedStringLen(wszName, be, YES);
    if (sl < 0) ERR_RET(EIV_WRONGFORMAT);
    
    nodeName = [NSString stringWithCString:wszName encoding:NSWindowsCP1252StringEncoding];
    [nodeArray addObject:nodeName];
    
    vnh = PAD_TO_32BIT((void*)vnh + vnh->cbNode);
    wszName = (char*)(vnh + 1);
  }
  
  return [nodeArray copy];
}


- (NSArray *)_query32BitSubNodesUnder:(NSString *)subBlock error:(EIVERSION_ERR *)err {
  NSData *queryNode;
  NSMutableArray *nodeArray;
  const void *bb, *be;
  const VERSIONNODE_HEADER *vnh;
  const unichar *wszName;
  NSString *nodeName;
  ssize_t nameLen;
  
  queryNode = EIResTreeRead32(EIRequestFromString(subBlock), 0, gBlock, err, YES);
  if (*err) return nil;
  
  nodeArray = [NSMutableArray array];
  vnh = bb = [queryNode bytes];
  be = bb + [queryNode length];
  
  wszName = (unichar *)(vnh + 1);
  while ((void*)vnh < be) {
    nameLen = EIUTF16CheckedStringLen(wszName, be, YES);
    if (nameLen < 0) ERR_RET(EIV_WRONGFORMAT);
    
    nodeName = [NSString stringWithCharacters:wszName length:nameLen];
    [nodeArray addObject:nodeName];
    
    vnh = PAD_TO_32BIT((void*)vnh + vnh->cbNode);
    wszName = (void*)(vnh + 1);
  }
  
  return [nodeArray copy];
}


@end
