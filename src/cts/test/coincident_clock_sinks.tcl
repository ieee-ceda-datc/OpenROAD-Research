# Distinct sinks can have identical clock-pin XY positions, as happens
# with matching upper/bottom-tier cells in a 3D owner view.
source "helpers.tcl"

foreach {name default} {sink_count 204 cluster_size 10 clustering 1} {
  if { ![info exists $name] } {
    set $name $default
  }
}

read_liberty Nangate45/Nangate45_typ.lib
read_lef Nangate45/Nangate45.lef

set db [ord::get_db]
set tech [$db getTech]
set chip [odb::dbChip_create $db $tech]
set block [odb::dbBlock_create $chip "coincident_clock_sinks"]
$block setDefUnits [$tech getDbUnitsPerMicron]
set area [odb::Rect]
$area init 0 0 800000 800000
$block setDieArea $area

set clk [odb::dbNet_create $block "clk"]
set term [odb::dbBTerm_create $clk "clk"]
$term setIoType INPUT
set pin [odb::dbBPin_create $term]
$pin setPlacementStatus FIRM
odb::dbBox_create $pin [$tech findLayer "metal5"] 0 0 100 100

set master [$db findMaster "DFF_X1"]
set original_sinks {}
for { set i 0 } { $i < $sink_count } { incr i } {
  set position $i
  if { $i == 1 } {
    set position 0
  }
  set x [expr {50000 + ($position % 30) * 20000}]
  set y [expr {50000 + ($position / 30) * 20000}]
  set inst [odb::dbInst_create $block $master "ff$i"]
  $inst setLocation $x $y
  $inst setPlacementStatus PLACED
  [$inst findITerm "CK"] connect $clk
  lappend original_sinks [list $inst $x $y]
}
sta::db_network_defined
create_clock -period 5 clk
set_wire_rc -clock -layer metal5
set_cts_config -wire_unit 20 \
  -distance_between_buffers 100 \
  -sink_clustering_size $cluster_size \
  -sink_clustering_max_diameter 60 \
  -sink_clustering_levels 1 \
  -num_static_layers 1 \
  -root_buf CLKBUF_X3 \
  -buf_list CLKBUF_X3

if { $clustering } {
  clock_tree_synthesis -sink_clustering_enable
} else {
  clock_tree_synthesis
}

# Check actual logical connections, not the reported geometric sink count.
foreach entry $original_sinks {
  lassign $entry inst x y
  set ck [$inst findITerm "CK"]
  set net [$ck getNet]
  if { $net eq "NULL" } {
    error "[$inst getName]/CK lost its clock connection"
  }
  set has_cts_driver 0
  foreach iterm [$net getITerms] {
    if { [$iterm isOutputSignal]
      && [string match "clkbuf_*" [[$iterm getInst] getName]] } {
      set has_cts_driver 1
    }
  }
  if { !$has_cts_driver } {
    error "[$inst getName]/CK has no CTS buffer driver"
  }
  if { [$inst getLocation] ne [list $x $y] } {
    error "CTS moved original sink [$inst getName]"
  }
}
puts "PASS: All $sink_count logical sinks are clocked at their original locations"
