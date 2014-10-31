

$fn=36;

/************************************************
	База - круглая шайба с углублением в центре.
*************************************************/

base_D = 38; // большой диаметр шайбы 
base_H = 3; // высота (толщина) шайбы
base_center_hole_D = 10; // диаметр центрального углубления
base_center_hole_H = 3; // глубина центрального углубления 

module base () {
	difference() {
		cylinder(d=base_D, h=base_H, center=true);
		cylinder(d=base_center_hole_D, h=base_center_hole_H, center=true);
	}
}
//base();

/************************************************
	Отверстие для крепления с фаской.
*************************************************/

mount_hole_D = 3; // диаметр отверстия 
mount_hole_had_D = 6; // диаметр фаски

module mount_hole (H,h) {
	union() {
		cylinder(d=mount_hole_D, h=H, center=true);
		translate([0,0,h]) { cylinder(d=mount_hole_had_D, h=H, center=true); }		
	}
}
//mount_hole(20, 3);

/************************************************
	Ухо - выступающее крепление.
*************************************************/

ear_D = 15; // ширина
ear_H = base_H; // высота (толщина)
ear_hole_D = 3; // диаметр отверстия

module ear () {
	difference() {
		union() {
			cylinder(d=ear_D, h=ear_H, center=true);
			translate([ear_D/2, 0,0]) { cube([ear_D, ear_D, ear_H], center=true); }
		}
		cylinder(d=ear_hole_D, h=ear_H+1, center=true);
	}
}
//ear();

/************************************************
	Выступающая часть для крепления трубы.
*************************************************/

tube_D = 15; // диаметр трубы
tube_mount_W = tube_D + 2*2; // ширина крепления для трубы (выступает с двух сторон на 2 миллиметра - поэтому "+ 2*2")
tube_mount_A = 5; // угол наклона трубы к горизонтальной плоскости

module tube_mount () {
	intersection() {
		difference() {
			cube([base_D, tube_mount_W, tube_D/2], center=true);
			translate([-base_D/2,0,0]) rotate([0,-tube_mount_A,0]) translate([base_D/2,0,0]) {
				translate([0,0,tube_D/4]) {
					rotate([0,90,0]) { 
						cylinder(d=tube_D, h=base_D*2, center=true);
					}
				}
			}
		}
		cylinder(d=base_D, h=tube_D, center=true);
	}
}
//tube_mount();


/************************************************
	А теперь - само крепление мотора.
*************************************************/

tube_mount_position_A = 0; // угол поворота крепления трубы относительно остальных элементов

ear_hole_position_R = 25; // расстояние от центра до отверстия в ухе
ear_position_A = 0; // угол поворота ушей относительно остальных элементов

mount_hole_position_R = 15; // расстояние от центра до отверстия для крепления
mount_hole_number = 3; // количество отверстий
mount_hole_position_A = 0; // угол поворота всех отверстий относительно остальных элементов

difference() {
	union() {
		rotate([0,0,tube_mount_position_A]) 
		translate([0,0,tube_D/4+base_H]) 
		{ 
			tube_mount();	
		}
		
		translate([0,0,base_H/2]) { base(); }
		
		rotate([0,0, ear_position_A]) 
		union() {
			translate([-ear_hole_position_R, 0, ear_H/2]) { ear(); }
			mirror([1,0,0]) { translate([-ear_hole_position_R, 0, ear_H/2]) { ear(); } }
		}
	}
	union() {
		for (i = [0 : mount_hole_number-1])
		{
			rotate([0,0, mount_hole_position_A + 360/mount_hole_number*i]) 
			translate([mount_hole_position_R, 0, tube_D]) 
			{ 
				mount_hole(tube_D*2, base_H); 
			}
		}
	}
}








