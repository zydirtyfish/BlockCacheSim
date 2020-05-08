
include proj.incl

CFLAGS+= -fPIC -DSNACC_DEEP_COPY -DHAVE_VARIABLE_SIZED_AUTOMATIC_ARRAYS -Wno-deprecated -g

INC+=  $(PROJ_INC)
OBJ+=$(PROJ_OBJ)

BIN=cache_sim
LIB+=$(PROJ_LIB)

$(BIN): $(OBJ)
	$(CXX) -lpthread $(CFLAGS)  -o $@ $^ $(LIB)

%.o: %.cpp
	$(CXX) -lpthread -std=gnu++0x $(CFLAGS) $(INC) -c -o $@ $< 
        
%.o: %.c
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<  

clean:
	rm -f $(PROJ_OBJ) $(BIN)

cleanall:
	rm -f $(OBJ)

test:
	@echo $(COMMON_OBJ)
