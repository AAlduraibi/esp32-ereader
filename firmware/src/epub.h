#pragma once

constexpr char MARK_BOLD = '\x01';
constexpr char MARK_ITALIC = '\x02';
constexpr char MARK_NORMAL = '\x03';


//bool epubTest(const char* name);

bool epubBuildCache(const char* name);

bool epubIsEpub(const char* name);