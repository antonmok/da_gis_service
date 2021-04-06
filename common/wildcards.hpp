#include <stdio.h>
#include <stdbool.h>
  
// Function that checks if two given strings
// match. The first string may contain wildcard characters
inline bool WildcardMatch(const char *pattern, const char *string)
{
    // If we reach at the end of both strings, we are done
    if (*pattern == '\0' && *string == '\0')
        return true;
  
    // Make sure that the characters after '*' are present
    // in second string. This function assumes that the first
    // string will not contain two consecutive '*'
    if (*pattern == '*' && *(pattern + 1) != '\0' && *string == '\0')
        return false;
  
    // If the first string contains '?', or current characters
    // of both strings match
    if (*pattern == '?' || *pattern == *string)
        return WildcardMatch(pattern + 1, string + 1);
  
    // If there is *, then there are two possibilities
    // a) We consider current character of second string
    // b) We ignore current character of second string.
    if (*pattern == '*')
        return WildcardMatch(pattern + 1, string) || WildcardMatch(pattern, string + 1);
    return false;
}