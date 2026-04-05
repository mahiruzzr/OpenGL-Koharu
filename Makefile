# 指定編譯器
CXX = g++

# 最終輸出的執行檔名稱
EXE = main

# 編譯時需要的標頭檔搜尋路徑 (-I.)
CXXFLAGS = -I. -I./imgui -I./imgui/backends -I/usr/local/include -I./glad_output/include -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM -Wall -Wformat

# 連結時需要的外部函式庫 (-l)
LIBS = -L/usr/local/lib -lglfw -lGL -lassimp

# 所有的原始碼檔案 (.cpp)
SOURCES = main.cpp \
          glad.c \
          imgui/imgui.cpp \
          imgui/imgui_draw.cpp \
          imgui/imgui_tables.cpp \
          imgui/imgui_widgets.cpp \
          imgui/backends/imgui_impl_glfw.cpp \
          imgui/backends/imgui_impl_opengl3.cpp

# 將 .cpp 副檔名替換成 .o (目的檔)
OBJS = $(SOURCES:.cpp=.o)

# 預設的編譯目標
all: $(EXE)
	@echo "編譯完成！"

# 如何把所有的 .o 檔連結成最終的執行檔 main
$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

# 如何把每一個 .cpp 檔編譯成 .o 檔
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# 清除編譯產生的暫存檔和執行檔
clean:
	rm -f $(EXE) $(OBJS)