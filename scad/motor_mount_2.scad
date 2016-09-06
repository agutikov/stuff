

$fn=60;


/************************************************
	Параметры.
*************************************************/

ball_D = 60; // диаметр шара
mount_D = 40; // диаметр маунта (диаметр основания мотора)
mount_hole_D = 3; // диаметр отверстия (толщина болтов)
mount_hole_D2 = 6; // диаметр углубления возле отверстия (диаметр шайбы)
mount_hole_pos_W = 25; // расстояние между отверстиями для крепления мотора
mount_center_hole_D = 10; // диаметр центрального углубления
mount_center_hole_H = 3; // глубина центрального углубления
tube_D1 = 8; // диаметр трубки вдоль луча
tube_D2 = 10; // диаметр трубки перпендикулярной лучу
tube_a1 = 4; // угол трубки вдоль луча
shaft_W = 14; // ширина отверстия для луча
shaft_H = 26; // высота отверстия для луча
shaft_d = 15; // расстояние от края сферы до конца луча
conn_H = 4; // толщина слоя между болтами соединяющими две половины маунта
conn_dr = 5; // расстояние от края сферы до отверстий для соединения полусфер
mount_layer_H = 4; // толщина слоя возле отверстия для крепления мотора



/************************************************
	Рассчётные параметры.
*************************************************/

mount_H = sqrt(pow((ball_D/2), 2) - pow((mount_D/2), 2)); // высота расположения плоскости маунта
mount_hole_pos_R = mount_hole_pos_W/(2*cos(30)); // радиус расположения отверстия для крепления мотора



/************************************************
	Модель.
*************************************************/

module hole (d1, d2, h1, h2) {
	union() {
		translate([0, 0, -1]) cylinder(d=d1, h=h1+1, center=false);
		translate([0, 0, -h2]) cylinder(d=d2, h=h2, center=false);
	}
}


module half_mount () {
	difference() {
		union() {
			sphere( d=ball_D );
/*
			difference() {
				scale([5.0, 2.0, 1.0]) sphere(d=20, center=true);
				translate([-50, 0, 0]) cube([100, 100, 100], center=true);
			}
			rotate([0, 0, tube_a1])
			difference() {
				scale([2.0, 5.0, 1.0]) sphere(d=20, center=true);
				translate([0, -50, 0]) cube([100, 100, 100], center=true);
			}
*/
		}
		union() {
			translate([0, 0, -50]) cube([100, 100, 100], center=true);
			translate([0, 0, 50 + mount_H]) cube([100, 100, 100], center=true);
			translate([0, -50 - mount_D/2, 0]) cube([100, 100, 100], center=true);
			
			translate([0, -50 - shaft_d + ball_D/2, 0]) cube([shaft_W, 100, shaft_H], center=true);
			
			translate([0, 0, 0]) rotate([0, 90, -tube_a1]) cylinder(d=tube_D2, h=100, center=false);
			translate([0, 0, 0]) rotate([-90, 0, 0]) cylinder(d=tube_D1, h=100, center=false);
			
			translate([0, 0, mount_H-mount_center_hole_H]) cylinder(d=mount_center_hole_D, h=mount_center_hole_H+10, center=false);
			
			rotate([0, 0, -90]) translate([mount_hole_pos_R, 0, mount_H - mount_layer_H]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			rotate([0, 0, 120 - 90]) translate([mount_hole_pos_R, 0, mount_H - mount_layer_H]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			rotate([0, 0, 240 - 90]) translate([mount_hole_pos_R, 0, mount_H - mount_layer_H]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			
			rotate([0, 0, 180 + 15]) translate([ball_D/2 - conn_dr, 0, conn_H]) mirror([0, 0, 1]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			rotate([0, 0, 120 + 180 + 15]) translate([ball_D/2 - conn_dr, 0, conn_H]) mirror([0, 0, 1]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			rotate([0, 0, 240 + 180 + 15]) translate([ball_D/2 - conn_dr, 0, conn_H]) mirror([0, 0, 1]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			
		}
	}
}

