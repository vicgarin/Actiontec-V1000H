#!/bin/bash

# -----------------------------------------------------------------------------
# ----------------- Variables -------------------------------------------------
# -----------------------------------------------------------------------------	

declare -a SHENTSIZE

MASKS="XA A WA SA"
MASKC=( X A W S )
MASKI=( 4 2 1 0 )
MASK2=( 0 1 0 0 )

coreSize=0
initSize=0

MODNAME="dspddTemp.ko"

# -----------------------------------------------------------------------------
# ----------------- Functions -------------------------------------------------
# -----------------------------------------------------------------------------	
masksMatch()
{
    local flagSum=0
	local maskSum=0

    local mask=$1
	local flags=$2

    for ((j=0; j < 4 ; j++))
    do
        if [ `echo "$mask" | sed s/.*${MASKC[$j]}.*/${MASKC[$j]}/g` = "${MASKC[$j]}" ]; then
		    maskSum=`expr $maskSum + ${MASKI[$j]}` 
		fi 

        if [ `echo "$flags" | sed s/.*${MASKC[$j]}.*/${MASKC[$j]}/g` = "${MASKC[$j]}" ]; then
		    flagSum=`expr $flagSum + ${MASKI[$j]}` 
		fi         
    done

    if [ "$4" = "1" -o "$5" = "1" ]; then
        if [ ${NAME[$3]} = ".symtab" -o ${NAME[$3]} = ".strtab" ]; then
            flagSum=$[$flagSum + ${MASKI[1]}]
	    fi
	fi

    if [ ${NAME[$3]} = ".modinfo" ]; then
        flagSum=$[$flagSum - ${MASKI[1]}]
	fi

    if [ "$flagSum" -eq "$maskSum" ]; then
        temp=$[$flagSum & ${MASK2[$k]}]	
        if [ "$temp" -eq "0" ]; then    
		    return 1
        fi
    fi

    return 0
}

getCoreOffset()
{
    local index=$1

    local exp2=$[${ALIGNMENT[$index]} - 1]
    local exp1=$[$coreSize + $exp2]
    local exp3=$[~$exp2]  
    local exp4=$[$exp1 & $exp3]    

	if [ "${ALIGNMENT[$index]}" -ne "0" ]; then
        SHENTSIZE[$index]=$exp4
    else
        SHENTSIZE[$index]=$[$coreSize & ~0]
	fi

    coreSize=$[${SHENTSIZE[$index]} + 0x${SIZE[$index]}]
    
    echo "Index=$index, Name=${NAME[$index]}, CoreSize=$coreSize, shSize=0x${SIZE[$index]}, EntSize=${SHENTSIZE[$index]}"                             
}

getInitOffset()
{
    local index=$1

    local exp2=$[${ALIGNMENT[$index]} - 1]
    local exp1=$[$initSize + $exp2]
    local exp3=$[~$exp2]  
    local exp4=$[$exp1 & $exp3]    

	if [ "${ALIGNMENT[$index]}" -ne "0" ]; then
        SHENTSIZE[$index]=$exp4
    else
        SHENTSIZE[$index]=$[$initSize & ~0]
	fi

    initSize=$[${SHENTSIZE[$index]} + 0x${SIZE[$index]}]                             
}

# -----------------------------------------------------------------------------
# ----------------- Main script -----------------------------------------------
# -----------------------------------------------------------------------------	

echo "********* Start **********"

`cp -f $1 ./$MODNAME`
#when KALLSYMS is enabled strip only debug symbols
if [ "$5" = "1" ]; then
   `$3strip --strip-debug ./$MODNAME`
else
   `$3strip --strip-unneeded ./$MODNAME`
fi

# --- Number of sections ---
SHNUM=`$3readelf -h $MODNAME | grep 'Number of section headers:' | awk '{print $5}'`

# --- Section name ---
NAME=( `$3readelf -S $MODNAME | grep '\[' | sed s/'\[.*]'//g | awk '{print $1}' | grep -v 'Name'`	)

# --- Section size ---
SIZE=( `$3readelf -S $MODNAME | grep '\[' | sed s/'\[.*]'//g | awk '{print $5}' | grep -v 'Size'` )

# --- Section flags ---
FLAGS=( `$3readelf -S $MODNAME | grep '\['| grep -v 'Flg' | sed s/'\[.*]'//g | awk '{print $7}' | sed s/'[0-9]'/0/g` )

# --- Section alignment ---
ALIGNMENT=( `$3readelf -S $MODNAME | grep '\[' | sed s/'\[.*]'//g | awk '{print $NF}' | grep -v 'Al'` )


# --- MAIN PROGRAM ---
echo "Debug mode: $4"
echo "KALLSYMS Enabled: $5"

for ((i=0; i < SHNUM ; i++))
do
    SHENTSIZE[$i]="NULL"
done     

# --- Get Core Size ---
k=0
for oneMask in `echo $MASKS`
do
    for ((i=0; i < SHNUM ; i++))
    do
        if [ "${NAME[$i]:0:5}" != ".init" ]; then
    	    masksMatch "$oneMask" "${FLAGS[$i]}" "$i" $4 $5	 
            if [ "$?" -eq "1" ]; then
			    if [ "${SHENTSIZE[$i]}" = "NULL" ]; then
                    getCoreOffset "$i"
				fi 
		    fi 
		fi 
    done
    k=$[$k+1]     
done

#--- Get Init Size ---
k=0
for oneMask in `echo $MASKS`
do
    for ((i=0; i < SHNUM ; i++))
    do
        if [ "${NAME[$i]:0:5}" = ".init" ]; then
    	    masksMatch "$oneMask" "${FLAGS[$i]}" "$i" $4 $5	 
            if [ "$?" -eq "1" ]; then
			    if [ "${SHENTSIZE[$i]}" = "NULL" ]; then
                    getInitOffset "$i"
				fi 
		    fi 
		fi 
    done     
    k=$[$k+1]     
done

echo "Core: $coreSize Init: $initSize"
`rm -f ./$MODNAME`

#--- Patch these values in special defines in init/main.c ---
`sed s/'#define DSP_CORE_SIZE .*'/"#define DSP_CORE_SIZE $coreSize"/g $2/opensource/include/bcm963xx/dsp_mod_size.h > $2/opensource/include/bcm963xx/dsp_mod_size1.h`
`mv -f $2/opensource/include/bcm963xx/dsp_mod_size1.h $2/opensource/include/bcm963xx/dsp_mod_size.h`
`sed s/'#define DSP_INIT_SIZE .*'/"#define DSP_INIT_SIZE $initSize"/g $2/opensource/include/bcm963xx/dsp_mod_size.h > $2/opensource/include/bcm963xx/dsp_mod_size1.h`
`mv -f $2/opensource/include/bcm963xx/dsp_mod_size1.h $2/opensource/include/bcm963xx/dsp_mod_size.h`

echo "********* End **********"
