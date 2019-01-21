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

/* Note: -stringWithCharacters:length: in NSString (like 
 * CFStringCreateWithCharacters, which is the function that actually does
 * the job under the hood) does NOT take a number of characters as length
 * parameter (as the documentation claims), but a number of CODE POINTS 
 * (bytes / 2). */

#import "EIVersionInfo.h"


#define PAD_TO_32BIT(x) (((void*)((x)) + ((4 - ((intptr_t)((x)) % 4)) % 4)))
#define ERR_RET(x) {*err = ((x)); return nil;}
#define CHECK_PTR(b, x, e) {if ((void*)(x) < (b) || (e) < (void*)(x)) ERR_RET(EIV_WRONGFORMAT);}


typedef struct {
  int16_t cbNode;  //Size of the node (a node includes its children)
  int16_t cbData;  //Size of the data in the node
  int16_t wType;   //1 if the node contains a string
} VERSIONNODE_HEADER;

typedef struct {
  int16_t cbNode;  //Size of the node (a node includes its children)
  int16_t cbData;  //Size of the data in the node
} VERSIONNODE16_HEADER;


/* Returns a number of code points */
int EIUTF16CheckedStringLen(const unichar* string, const unichar *maxptr, BOOL expectTerm)
{
  const unichar *initial;
  
  for (initial = string; ((void*)string) + 1 < (void*)maxptr; string++)
    if (*string == '\0')
      return (int)(string - initial);
  return expectTerm ? -1 : (int)(string - initial);
}


int EICheckedStringLen(const char *str, const char *maxptr, BOOL expectTerm)
{
  const char *initial;
  
  for (initial = str; str < maxptr; str++)
    if (*str == '\0')
      return (int)(str - initial);
  return expectTerm ? -1 : (int)(str - initial);
}


NSArray *EIRequestFromString(NSString *subBlock)
{
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


const VERSIONNODE_HEADER *EIResTreeIterateChildren32(NSData *block, EIVERSION_ERR *err,
  void (^callb)(BOOL *stop, NSString *name, const void *dataptr))
{
  BOOL found;
  const void *bb, *be;
  const VERSIONNODE_HEADER *vnh;
  const unichar *wszName;
  const void *dataptr;
  ssize_t nameLen;
  NSString *nodeName;
  
  vnh = bb = [block bytes];
  be = bb + [block length];
  *err = EIV_NOERR;
  
  wszName = (unichar *)(vnh + 1);
  
  found = NO;
  while ((void*)vnh < be) {
    nameLen = EIUTF16CheckedStringLen(wszName, be, YES);
    if (nameLen < 0)
      ERR_RET(EIV_WRONGFORMAT);
    
    nodeName = [NSString stringWithCharacters:wszName length:nameLen];
    dataptr = PAD_TO_32BIT((void*)(wszName + nameLen + 1));
    
    callb(&found, nodeName, dataptr);
    if (found)
      return vnh;
    
    vnh = PAD_TO_32BIT((void*)vnh + vnh->cbNode);
    wszName = (const unichar*)(vnh + 1);
  }
  return NULL;
}


NSData *EIResTreeRead32(NSArray *path, int level, NSData *block,
  EIVERSION_ERR *err, BOOL getChildren)
{
  const VERSIONNODE_HEADER *vnh;
  ssize_t subtreeLen;
  const void __block *dataptr;
  const void *nextptr;
  NSString *nodeToRead;
  NSData *nodeData;
  
  nodeToRead = [path objectAtIndex:level];
  vnh = EIResTreeIterateChildren32(block, err, ^(BOOL *stop, NSString *name, const void *dp) {
    dataptr = dp;
    *stop = [name caseInsensitiveCompare:nodeToRead] == NSOrderedSame ||
            [@"*" isEqual:nodeToRead];
  });
  if (*err)
    return NULL;
  if (!vnh)
    ERR_RET(EIV_UNKNOWNNODE);
  
  nextptr = PAD_TO_32BIT(dataptr + vnh->cbData);  //pointer to first child node
  subtreeLen = ((void*)vnh + vnh->cbNode) - nextptr;
  
  CHECK_PTR([block bytes], nextptr + subtreeLen - 1, [block bytes] + [block length]);
  
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


const VERSIONNODE16_HEADER *EIResTreeIterateChildren16(NSData *block, EIVERSION_ERR *err,
  void (^callb)(BOOL *stop, NSString *name, const void *dataptr))
{
  BOOL found;
  const void *bb, *be;
  const VERSIONNODE16_HEADER *vnh;
  const char *wszName;
  ssize_t sl;
  const void *dataptr;
  NSString *nodeName;
  
  vnh = bb = [block bytes];
  be = bb + [block length];
  *err = EIV_NOERR;
  
  wszName = (char*)(vnh + 1);
  
  found = NO;
  while ((void*)vnh < be) {
    sl = EICheckedStringLen(wszName, be, YES);
    if (sl < 0)
      ERR_RET(EIV_WRONGFORMAT);
    
    nodeName = [NSString stringWithCString:wszName encoding:NSWindowsCP1252StringEncoding];
    dataptr = PAD_TO_32BIT(wszName + [nodeName length] + 1);
    
    callb(&found, nodeName, dataptr);
    if (found)
      return vnh;
    
    vnh = PAD_TO_32BIT((void*)vnh + vnh->cbNode);
    wszName = (const char*)(vnh + 1);
  }
  return NULL;
}


NSData *EIResTreeRead16(NSArray *path, int level, NSData* block,
  EIVERSION_ERR *err, BOOL getChildren)
{
  const VERSIONNODE16_HEADER *vnh;
  ssize_t subtreeLen;
  const void __block *dataptr;
  const void *nextptr;
  NSString *nodeToRead;
  NSData *nodeData;
  
  nodeToRead = [path objectAtIndex:level];
  
  vnh = EIResTreeIterateChildren16(block, err, ^(BOOL *stop, NSString *name, const void *dp) {
    dataptr = dp;
    *stop = [name caseInsensitiveCompare:nodeToRead] == NSOrderedSame ||
            [@"*" isEqual:nodeToRead];
  });
  if (*err)
    return NULL;
  if (!vnh)
    ERR_RET(EIV_UNKNOWNNODE);
  
  nextptr = PAD_TO_32BIT(dataptr + vnh->cbData);  //pointer to first child node
  subtreeLen = ((void*)vnh + vnh->cbNode) - nextptr;
  
  CHECK_PTR([block bytes], nextptr + subtreeLen - 1, [block bytes] + [block length]);
  
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


- (instancetype)initWithData:(NSData *)myBlock is16Bit:(BOOL)win16
{
  gBlock = myBlock;
  win16Block = win16;
  return [super init];
}


- (BOOL)is16bit
{
  return win16Block;
}


- (NSData *)data
{
  return gBlock;
}


- (NSString *)queryStringValue:(NSString *)subBlock error:(EIVERSION_ERR *)err
{
  NSData *raw;
  const void *bytes;
  int l;
  
  raw = [self queryValue:subBlock error:err];
  if (!raw)
    return nil;
  bytes = [raw bytes];
  
  if (win16Block) {
    l = EICheckedStringLen(bytes, (const char *)(bytes + [raw length]), NO);
    return [[NSString alloc] initWithBytes:bytes length:l
            encoding:NSWindowsCP1252StringEncoding];
  }
  
  l = EIUTF16CheckedStringLen(bytes, (const unichar *)(bytes + [raw length]), NO);
  return [NSString stringWithCharacters:bytes length:l];
}


// Like VerQueryValue(pBlock, lpSubBlock, lplpBuffer, puLen);
- (NSData *)queryValue:(NSString *)subBlock error:(EIVERSION_ERR *)err
{
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


- (NSArray *)querySubNodesUnder:(NSString *)subBlock error:(EIVERSION_ERR *)err
{
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


- (NSArray *)_query16BitSubNodesUnder:(NSString *)subBlock error:(EIVERSION_ERR *)err
{
  NSData *queryNode;
  NSMutableArray *nodeArray;
  
  queryNode = EIResTreeRead16(EIRequestFromString(subBlock), 0, gBlock, err, YES);
  if (*err)
    return nil;
  
  nodeArray = [NSMutableArray array];
  EIResTreeIterateChildren16(queryNode, err, ^(BOOL *stop, NSString *name, const void *dataptr) {
    [nodeArray addObject:name];
  });
  if (*err)
    return nil;
  
  return [nodeArray copy];
}


- (NSArray *)_query32BitSubNodesUnder:(NSString *)subBlock error:(EIVERSION_ERR *)err
{
  NSData *queryNode;
  NSMutableArray *nodeArray;
  
  queryNode = EIResTreeRead32(EIRequestFromString(subBlock), 0, gBlock, err, YES);
  if (*err)
    return nil;
  
  nodeArray = [NSMutableArray array];
  EIResTreeIterateChildren32(queryNode, err, ^(BOOL *stop, NSString *name, const void *dataptr) {
    [nodeArray addObject:name];
  });
  if (*err)
    return nil;
  
  return [nodeArray copy];
}


@end
