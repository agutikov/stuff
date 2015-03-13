

$fn=36;

/************************************************
	База - круглая шайба с углублением в центре.
*************************************************/

base_D = 42; // большой диаметр шайбы 
base_H = 3; // высота (толщина) шайбы
base_center_hole_D = 10; // диаметр центрального углубления
base_center_hole_H = 3; // глубина центрального углубления 

module base () {
	cylinder(d=base_D, h=base_H, center=true);
}
//base();

/************************************************
	Отверстие для крепления с фаской.
*************************************************/

mount_hole_D = 3; // диаметр отверстия 
mount_hole_had_D = 6; // диаметр фаски

/*
module mount_hole (H,h) {
	union() {
		cylinder(d=mount_hole_D, h=H, center=true);
		translate([0,0,h]) { cylinder(d=mount_hole_had_D, h=H, center=true); }		
	}
}
*/
//mount_hole(20, 3);

module mount_hole (H,h) {
	union() {
		cylinder(d=mount_hole_D, h=H, center=true);	
	}
}

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

tube_D = 12.5; // диаметр трубы
tube_mount_W = tube_D + 3*2; // ширина крепления для трубы (выступает с двух сторон на 2 миллиметра - поэтому "+ 2*2")
tube_mount_A = 0; // угол наклона трубы к горизонтальной плоскости
tube_mount_H = tube_mount_W / 2; // высота крепления трубы

module tube_mount () {
	intersection() {
		difference() {
			translate([0,0,tube_D/4])
				cube([base_D*2, tube_mount_W, tube_mount_W], center=true);
			union() {
				translate([0,0,tube_D/4 + tube_mount_H])
					cube([base_D*2, tube_mount_W * 2, tube_mount_W], center=true);
				translate([-base_D/2,0,0]) rotate([0,-tube_mount_A,0]) translate([base_D/2,0,0]) {
					translate([0,0,tube_D/4]) {
						rotate([0,90,0]) { 
							cylinder(d=tube_D, h=base_D*2, center=true);
						}
					}
				}
			}
		}
		union() {
			cylinder(d=base_D, h=tube_D*2, center=true);
			translate([3,0,tube_D/4])
				cube([base_D, tube_mount_W, tube_mount_W], center=true);
		}
	}
}
//tube_mount();

/************************************************
	Выступающая часть для крепления к фанере.
*************************************************/

p_W = 16; // ширина фанеры
p_whole_W = p_W + base_H*2; // ширина крепления для фанеры (выступает с двух сторон на 2 миллиметра - поэтому "+ 2*2")
p_A = 0; // угол наклона фанеры к горизонтальной плоскости
p_H = 12; // высота крепления фанеры

module p_mount () {
	intersection() {
		difference() {
			translate([0,0,p_W/4])
				cube([base_D*2, p_whole_W, p_whole_W], center=true);
			union() {
				translate([0,0,base_H + p_H])
					cube([base_D*2, p_whole_W * 2, p_whole_W], center=true);
				translate([-base_D/2,0,0]) rotate([0,-p_A,0]) translate([base_D/2,0,0]) {
					translate([0,0,p_W/4]) {
						rotate([0,90,0]) { 
							cube([p_W, p_W, base_D*2], center=true);
						}
					}
				}
			}
		}
		union() {
			cylinder(d=base_D, h=p_W*2, center=true);
			translate([3,0,p_W/4])
				cube([base_D, p_whole_W, p_whole_W], center=true);
		}
	}
}
//p_mount();


/************************************************
	А теперь - само крепление мотора.
*************************************************/

tube_mount_position_A = 0; // угол поворота крепления трубы относительно остальных элементов

ear_hole_position_R = 28; // расстояние от центра до отверстия в ухе
ear_position_A = 28; // угол поворота ушей относительно остальных элементов
// ear_position_A = -28; // зеркальное положение ушей

mount_A = 28;
mount_hole_position_R = mount_A/2/cos(30); // расстояние от центра до отверстия для крепления
mount_hole_number = 3; // количество отверстий
mount_hole_position_A = 0; // угол поворота всех отверстий относительно остальных элементов


difference() {
	union() {
		rotate([0,0,tube_mount_position_A]) 
		translate([0,0,p_W/4+base_H]) 
		{ 
			p_mount();	
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
		rotate([0,0,-60])
		translate([base_D/2,0,0])
			cylinder(d=12, h=base_H);

		translate([0,0,0])
			cylinder(d=base_center_hole_D, h=base_center_hole_H, center=true);
	}
}







