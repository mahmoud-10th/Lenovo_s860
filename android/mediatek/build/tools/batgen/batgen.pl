#!/usr/local/bin/perl -w
#
#****************************************************************************/
#*
#* Filename:
#* ---------
#*   ptgen.pl
#*
#* Project:
#* --------
#*
#*
#* Description:
#* ------------
#*   This script will generate partition layout files
#*        
#*
#* Author: Kai Zhu (MTK81086)
#* -------
#*
#* 
#*============================================================================
#*             HISTORY
#* Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
#*------------------------------------------------------------------------------
#* $Revision$
#* $Modtime$
#* $Log$
#*
#*
#*------------------------------------------------------------------------------
#* Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
#*============================================================================
#****************************************************************************/

#****************************************************************************
# Included Modules
#****************************************************************************
use File::Basename;
my $Version=1.0;

my $ChangeHistory = "1.0 create battery parameter\n";

my $FIRST_OCV               = 1;
my $FIRST_DOD             = $FIRST_OCV + 4;
my $FIRST_R             = $FIRST_OCV + 5;
my $COL_NUM             = 7;

my $FIRST_TEMP               = 0;
my $FIRST_NTC             = 1;

my $FIRST_PRAMAS_DEFINE      = 3;
my $FIRST_PRAMAS             = 4;

my $column1;
my $column2;
my $num = 0;
my $SHEET_NAME = "ZCV";
my $NTC_SHEET_NAME = "NTC";
my $FINISH = "end";
my $zcv_dod;
my $zcv_ocv;
my $rownum;
my $DOD = 1;
my $R = 1;
my $NTC = 1;

BEGIN
{
  $LOCAL_PATH = dirname($0);
}

use lib "$LOCAL_PATH/../Spreadsheet";
use lib "$LOCAL_PATH/../";
require 'ParseExcel.pm';


my $ZCV_FILENAME   = "mediatek/build/tools/batgen/zcv_$ENV{'MTK_PROJECT'}.xls"; # excel file name
my $ZCV_DEFINE_H_NAME     = "mediatek/custom/$ENV{'MTK_PROJECT'}/kernel/battery/battery/cust_battery_meter_table.h"; # 

#****************************************************************************
# main thread
#****************************************************************************
# get already active Excel application or open new
print "*******************Arguments*********************\n" ;
print "Version=$Version ChangeHistory:$ChangeHistory\n";
print "PLATFORM = $ENV{MTK_PLATFORM};
PROJECT = $ENV{PROJECT};
ZCV_FILENAME = $ZCV_FILENAME;
ZCV_DEFINE_H_NAME=$ZCV_DEFINE_H_NAME;
\n";
print "*******************Arguments*********************\n\n\n\n" ;

$ZcvBook = Spreadsheet::ParseExcel->new()->Parse($ZCV_FILENAME);

&GenHeaderFile () ;


print "**********Batgen Done********** ^_^\n" ;
print "\n\Batgen modified or Generated files list:\n$ZCV_DEFINE_H_NAMEE\n";

exit ;

#****************************************************************************************
# subroutine:  get_sheet
# return:      Excel worksheet no matter it's in merge area or not, and in windows or not
# input:       Specified Excel Sheetname
#****************************************************************************************
sub get_sheet {
  my ($sheetName,$Book) = @_;
  return $Book->Worksheet($sheetName);
}

#****************************************************************************
# subroutine:  GenHeaderFile
# return:      
#****************************************************************************
sub checkNumeric {
    my ($value1, $value2) = @_;
    unless($value1 eq '0') {
        my $temp = $value1+0;
        if($temp == 0) {
            die "value isn't numeric";
        }
    }

    unless($value2 eq '0') {
        $temp = $value2+0;
        if($temp == 0) {
            die "value isn't numeric";
        }
    }

 }

sub round
{
    my ($value,$rank)=@_;
    if($value>0)
    {
        return int($value*10**$rank+0.5)/10**$rank;
    }
    else
    {
        return int($value*10**$rank-0.4)/10**$rank;
    }
}

sub GenDefineParameter {
    my ($Sheet, $row, $col1, $col2, $SheetName) = @_;
    my $value1 = &xls_cell_value($Sheet, $row, $col1, $SheetName);
    my $value2;
    $num = 0;
    while($value1 ne $FINISH) {
        $num++;
        $value2 = &xls_cell_value($Sheet, $row, $col2,$SheetName);
        print  ZCV_DEFINE_H_NAME "#define ";
        print  ZCV_DEFINE_H_NAME "$value2";
        print  ZCV_DEFINE_H_NAME "             ";
            
        print  ZCV_DEFINE_H_NAME "$value1";
        print  ZCV_DEFINE_H_NAME "\n";
            
        $row++;
        $value1 = &xls_cell_value($Sheet, $row, $col1, $SheetName);
    }
}

sub GenParameter {
    my ($Sheet, $row, $col1, $col2, $SheetName) = @_;
    my $value1 = &xls_cell_value($Sheet, $row, $col1, $SheetName);
    my $value2;
    $num = 0;
    while($value1 ne $FINISH) {
        $num++;
        $value2 = &xls_cell_value($Sheet, $row, $col2,$SheetName);
        checkNumeric($value1,$value2);
        $value1 = round($value1, 0);
        $value2 = round($value2, 0);
        print  ZCV_DEFINE_H_NAME "{";
        print  ZCV_DEFINE_H_NAME "$value2";
        print  ZCV_DEFINE_H_NAME ", ";
            
        print  ZCV_DEFINE_H_NAME "$value1";
        print  ZCV_DEFINE_H_NAME "}";
            
        $row++;
        $value1 = &xls_cell_value($Sheet, $row, $col1, $SheetName);
        if($value1 ne $FINISH) {
            print ZCV_DEFINE_H_NAME ",";
        }    
        print  ZCV_DEFINE_H_NAME "\n";
    }
    print  ZCV_DEFINE_H_NAME "};\n\n";
}

sub GenParameterDod {
    my ($Sheet, $row, $col1, $col2, $SheetName) = @_;
    my $value1 = &xls_cell_value($Sheet, $row, $col1, $SheetName);
    my $value2;
    $num = 0;
    while($value1 ne $FINISH) {
        $num++;
        $value2 = &xls_cell_value($Sheet, $row, $col2,$SheetName);
        checkNumeric($value1,$value2);
        $value1 = round($value1, 0);
        $value2 = round($value2, 0);
        my $temp = $value2  + 0;
        if($temp > 100) {
            $temp = 100;
        }
        $value2 = sprintf("%.0f",$temp);
        print  ZCV_DEFINE_H_NAME "{";
        print  ZCV_DEFINE_H_NAME "$value2";
        print  ZCV_DEFINE_H_NAME ", ";
            
        print  ZCV_DEFINE_H_NAME "$value1";
        print  ZCV_DEFINE_H_NAME "}";
            
        $row++;
        $value1 = &xls_cell_value($Sheet, $row, $col1, $SheetName);
        if($value1 ne $FINISH) {
            print ZCV_DEFINE_H_NAME ",";
        }    
        print  ZCV_DEFINE_H_NAME "\n";
    }
    print  ZCV_DEFINE_H_NAME "};\n\n";
}

sub GenDefine_1()
{
    	my $template = <<"__TEMPLATE";
#ifndef _CUST_BATTERY_METER_TABLE_H
#define _CUST_BATTERY_METER_TABLE_H

#include <mach/mt_typedefs.h>

// ============================================================
// define
// ============================================================
__TEMPLATE
    return $template;
}

sub GenDefine_2()
{
my $template = <<"__TEMPLATE";



// ============================================================
// ENUM
// ============================================================

// ============================================================
// structure
// ============================================================

// ============================================================
// typedef
// ============================================================
typedef struct _BATTERY_PROFILE_STRUC
{
    kal_int32 percentage;
    kal_int32 voltage;
} BATTERY_PROFILE_STRUC, *BATTERY_PROFILE_STRUC_P;

typedef struct _R_PROFILE_STRUC
{
    kal_int32 resistance; // Ohm
    kal_int32 voltage;
} R_PROFILE_STRUC, *R_PROFILE_STRUC_P;

typedef enum
{
    T1_0C,
    T2_25C,
    T3_50C
} PROFILE_TEMPERATURE;

// ============================================================
// External Variables
// ============================================================

// ============================================================
// External function
// ============================================================

// ============================================================
// <DOD, Battery_Voltage> Table
// ============================================================
__TEMPLATE
    return $template;
}

sub GenDefineFun() {
    	my $template = <<"__TEMPLATE";
// ============================================================
// function prototype
// ============================================================
int fgauge_get_saddles(void);
BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature);

int fgauge_get_saddles_r_table(void);
R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature);
__TEMPLATE
    return $template;
}
sub GenHeaderFile ()
{
    my $iter = 0 ;
    my $temp ;
    my $t;
    unless(-e $ZCV_FILENAME) {
        print("ZCV_FILENAME not find\n ");
        return;
    }

    my $sheet = get_sheet($SHEET_NAME,$ZcvBook);
    my $ntc_sheet = get_sheet($NTC_SHEET_NAME,$ZcvBook);
    
    unless ($sheet) {
        my $error_msg="Ptgen CAN NOT find sheet=$SHEET_NAME in $ZCV_TABLE_FILENAME\n";
        print $error_msg;
        return;
        #die $error_msg;
    }

    unless ($ntc_sheet) {
        my $error_msg="Ptgen CAN NOT find sheet=$NTC_SHEET_NAME in $ZCV_TABLE_FILENAME\n";
        print $error_msg;
        return;
        #die $error_msg;
    }
    if (-e $ZCV_DEFINE_H_NAME) {
        `chmod 777 $ZCV_DEFINE_H_NAME`;
    }
    
    open (ZCV_DEFINE_H_NAME, ">$ZCV_DEFINE_H_NAME") or &error_handler("Batgen open ZCV_DEFINE_H_NAME fail!\n", __FILE__, __LINE__);
    my $gen_define1 = GenDefine_1();
    print ZCV_DEFINE_H_NAME $gen_define1;

#gen define
    $rownum = $NTC;
    $column1 = $FIRST_PRAMAS_DEFINE ;
    $column2 = $FIRST_PRAMAS;
    GenDefineParameter($ntc_sheet, $rownum, $column2, $column1, $SHEET_NAME);

    my $gen_define2 = GenDefine_2();
    print ZCV_DEFINE_H_NAME $gen_define2;
#NTC 
    print  ZCV_DEFINE_H_NAME "BATT_TEMPERATURE Batt_Temperature_Table[] = {\n";
    $rownum = $NTC;
    $column1 = $FIRST_TEMP ;
    $column2 = $FIRST_NTC;
    GenParameter($ntc_sheet, $rownum, $column2, $column1, $SHEET_NAME);

#-10c doc    
    print  ZCV_DEFINE_H_NAME "// T0 -10C\nBATTERY_PROFILE_STRUC battery_profile_t0[] =\n{\n";
    $rownum = $DOD;
    $column1 = $FIRST_DOD + $COL_NUM*3 ;
    $column2 = $FIRST_OCV + $COL_NUM*3;
    GenParameterDod($sheet, $rownum, $column2, $column1, $SHEET_NAME);
    
#0c doc  
    print  ZCV_DEFINE_H_NAME "// T1 0C\nBATTERY_PROFILE_STRUC battery_profile_t1[] =\n{\n";
    $column1 = $column1 - $COL_NUM ;
    $column2 = $column2 - $COL_NUM;
    GenParameterDod($sheet, $rownum, $column2, $column1, $SHEET_NAME);
#25c doc    
    print  ZCV_DEFINE_H_NAME "// T2 25C\nBATTERY_PROFILE_STRUC battery_profile_t2[] =\n{\n";
    $column1 = $column1 - $COL_NUM ;
    $column2 = $column2 - $COL_NUM;
   GenParameterDod($sheet, $rownum, $column2, $column1, $SHEET_NAME);
   
#50c doc
    print  ZCV_DEFINE_H_NAME "// T3 50C\nBATTERY_PROFILE_STRUC battery_profile_t3[] =\n{\n";
    $column1 = $column1 - $COL_NUM ;
    $column2 = $column2 - $COL_NUM;
    GenParameterDod($sheet, $rownum, $column2, $column1, $SHEET_NAME);

#temp doc
    print  ZCV_DEFINE_H_NAME "// battery profile for actual temperature. The size should be the same as T1, T2 and T3\nBATTERY_PROFILE_STRUC battery_profile_temperature[] =\n{\n";
    my $index = 0;
    for($index=0; $index < $num; $index++) {
        print  ZCV_DEFINE_H_NAME "{0, 0}";
        if($index ne ($num-1)) {
            print  ZCV_DEFINE_H_NAME ",";
        }
        print  ZCV_DEFINE_H_NAME "\n";
    }
   print  ZCV_DEFINE_H_NAME "};\n\n";

#-10c r
    print  ZCV_DEFINE_H_NAME "// ============================================================
// <Rbat, Battery_Voltage> Table
// ============================================================
// T0 -10C\nR_PROFILE_STRUC r_profile_t0[] =\n{\n";
    $rownum = $R;
    $column1 = $FIRST_R + $COL_NUM*3 ;
    $column2 = $FIRST_OCV + $COL_NUM*3;
    GenParameter($sheet, $rownum, $column2, $column1, $SHEET_NAME);

#0c r    
    print  ZCV_DEFINE_H_NAME "// T1 0C\nR_PROFILE_STRUC r_profile_t1[] =\n{\n";
    $column1 = $column1 - $COL_NUM ;
    $column2 = $column2 - $COL_NUM;
    GenParameter($sheet, $rownum, $column2, $column1, $SHEET_NAME);

#25c r    
    print  ZCV_DEFINE_H_NAME "// T2 25C\nR_PROFILE_STRUC r_profile_t2[] =\n{\n";
    $column1 = $column1 - $COL_NUM ;
    $column2 = $column2 - $COL_NUM;
    GenParameter($sheet, $rownum, $column2, $column1, $SHEET_NAME);
    
#50c r
    print  ZCV_DEFINE_H_NAME "// T3 50C\nR_PROFILE_STRUC r_profile_t3[] =\n{\n";
    $column1 = $column1 - $COL_NUM ;
    $column2 = $column2 - $COL_NUM;
    GenParameter($sheet, $rownum, $column2, $column1, $SHEET_NAME);

#temp r
    print  ZCV_DEFINE_H_NAME "// r-table profile for actual temperature. The size should be the same as T1, T2 and T3\nR_PROFILE_STRUC r_profile_temperature[] =\n{\n";
    my $index = 0;
    for($index=0; $index < $num; $index++) {
        print  ZCV_DEFINE_H_NAME "{0, 0}";
        if($index ne ($num-1)) {
            print  ZCV_DEFINE_H_NAME ",";
        }
        print  ZCV_DEFINE_H_NAME "\n";
    }
    print  ZCV_DEFINE_H_NAME "};\n";
    $gen_define = GenDefineFun();
    print ZCV_DEFINE_H_NAME $gen_define;
    print  ZCV_DEFINE_H_NAME "\n#endif	//#ifndef _CUST_BATTERY_METER_TABLE_H";
    print  ZCV_DEFINE_H_NAME "\n ";
}

#****************************************************************************
# subroutine:  error_handler
# input:       $error_msg:     error message
#****************************************************************************
sub error_handler()
{
	   my ($error_msg, $file, $line_no) = @_;
	   my $final_error_msg = "Ptgen ERROR: $error_msg at $file line $line_no\n";
	   print $final_error_msg;
	   die $final_error_msg;
}

#****************************************************************************************
# subroutine:  xls_cell_value
# return:      Excel cell value no matter it's in merge area or not, and in windows or not
# input:       $Sheet:  Specified Excel Sheet
# input:       $row:    Specified row number
# input:       $col:    Specified column number
#****************************************************************************************
sub xls_cell_value {
	my ($Sheet, $row, $col,$SheetName) = @_;
	my $cell = $Sheet->get_cell($row, $col);
	if(defined $cell){
		return  $cell->Value();
  	}else{
		my $error_msg="ERROR in ptgen.pl: (row=$row,col=$col) undefine in $SheetName!\n";
		die $error_msg;
		return $FINISH;
	}
}
