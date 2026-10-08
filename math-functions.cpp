#include <math.h>

#include "../universal-features/error.h"


double degreesToRadians(double degrees) {

    return degrees * M_PI / 180.0;
}

ErrorCode addCommand(stack_t* stack){

    if(stack == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    stackElem_t value1 = 0;
    stackElem_t value2 = 0;

    stackPop(stack, &value1);
    stackPop(stack, &value2);
    stackPush(stack, value1 + value2);

    return ERR_OK;
}

ErrorCode subCommand(stack_t* stack){

    if(stack == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    stackElem_t value1 = 0;
    stackElem_t value2 = 0;

    stackPop(stack, &value1);
    stackPop(stack, &value2);
    stackPush(stack, value1 - value2);

    return ERR_OK;
}

ErrorCode mulCommand(stack_t* stack){

    if(stack == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    stackElem_t value1 = 0;
    stackElem_t value2 = 0;

    stackPop(stack, &value1);
    stackPop(stack, &value2);
    stackPush(stack, (stackElem_t)((double)value1 * value2 / 1000));

    return ERR_OK;
}

ErrorCode divCommand(stack_t* stack){

    if(stack == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    stackElem_t value1 = 0;
    stackElem_t value2 = 0;

    stackPop(stack, &value1);
    stackPop(stack, &value2);

    double result = (double)value2 / (double)value1;

    stackPush(stack, (stackElem_t)(result * 1000));

    return ERR_OK;
}

ErrorCode squareCommand(stack_t* stack){

    if(stack == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    stackElem_t value = 0;

    stackPop(stack, &value);

    double result = sqrt((double)value / 1000);

    stackPush(stack, (stackElem_t)(result * 1000));

    return ERR_OK;
}

ErrorCode sinCommand(stack_t* stack){

    if(stack == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    stackElem_t value = 0;

    stackPop(stack, &value);

    double degrees = (double)value / 1000;
    double result = sin(degreesToRadians(degrees));
    stackPush(stack, (stackElem_t)(result * 1000));

    return ERR_OK;

}

ErrorCode cosCommand(stack_t* stack){

    if(stack == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    stackElem_t value = 0;

    stackPop(stack, &value);

    double degrees = (double)value / 1000;
    double result = cos(degreesToRadians(degrees));
    stackPush(stack, (stackElem_t)(result * 1000));

    return ERR_OK;

}

ErrorCode tgCommand(stack_t* stack){

    if(stack == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    stackElem_t value = 0;

    stackPop(stack, &value);

    double degrees = (double)value / 1000;
    double result = tan(degreesToRadians(degrees));
    stackPush(stack, (stackElem_t)(result * 1000));

    return ERR_OK;

}

ErrorCode ctgCommand(stack_t* stack){

    if(stack == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    stackElem_t value = 0;

    stackPop(stack, &value);

    double degrees = (double)value / 1000;
    double result = 1 / tan(degreesToRadians(degrees));
    stackPush(stack, (stackElem_t)(result * 1000));

    return ERR_OK;

}
