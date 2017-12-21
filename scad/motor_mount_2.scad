

$fn=60;


/************************************************
	Параметры.
*************************************************/

ball_D = 60; // диаметр шара
shaft_W = 16; // ширина отверстия для луча
shaft_H = 15; // высота отверстия для луча
mount_H = shaft_H/2 + 8; // высота расположения маунта (основания мотора)
mount_hole_D = 3; // диаметр отверстия (толщина болтов)
mount_hole_D2 = 6; // диаметр углубления возле отверстия (диаметр шайбы)
mount_hole_pos_W = 28; // расстояние между отверстиями для крепления мотора
mount_center_hole_D = 10; // диаметр центрального углубления
mount_center_hole_H = 3; // глубина центрального углубления
tube_D1 = 10.6; // диаметр трубки вдоль луча
tube_D2 = 10.6; // диаметр трубки перпендикулярной лучу
tube_a1 = 0; // угол трубки вдоль луча
shaft_d = 15; // расстояние от края сферы до конца луча
conn_H = 4; // толщина слоя между болтами соединяющими две половины маунта
conn_dr = 7; // расстояние от края сферы до отверстий для соединения полусфер
mount_layer_H = 4; // толщина слоя возле отверстия для крепления мотора



/************************************************
	Рассчётные параметры.
*************************************************/

mount_hole_pos_R = (mount_hole_pos_W/2)/cos(30); // радиус расположения отверстия для крепления мотора
mount_R = sqrt(pow(ball_D/2, 2) - pow(mount_H, 2)); // радиус площадки маунта


/************************************************
	Модель.
*************************************************/

module hole (d1, d2, h1, h2) {
	union() {
		translate([0, 0, -1]) cylinder(d=d1, h=h1+1, center=false);
		translate([0, 0, -h2]) cylinder(d=d2, h=h2, center=false);
	}
}

A2 = 45;

module half_mount () {
	difference() {
		union() {
			sphere( d=ball_D );
/*
			rotate([0, 0, -tube_a1])
			difference() {
				scale([5.0, 2.0, 1.5]) sphere(d=20, center=true);
				translate([-50, 0, 0]) cube([100, 100, 100], center=true);
			}
			difference() {
				scale([2.0, 5.0, 1.0]) sphere(d=20, center=true);
				translate([0, -50, 0]) cube([100, 100, 100], center=true);
			}
*/
		}
		union() {
			translate([0, 0, -50]) cube([100, 100, 100], center=true);
			translate([0, 0, 50 + mount_H]) cube([100, 100, 100], center=true);
			translate([0, -50 - mount_R, 0]) cube([100, 100, 100], center=true);
			
			translate([0, -50 - shaft_d + ball_D/2, 0]) cube([shaft_W, 100, shaft_H], center=true);
			
			translate([-50, 0, 0]) rotate([0, 90, -tube_a1]) cylinder(d=tube_D2, h=100, center=false);
			translate([0, 0, 0]) rotate([-90, 0, 0]) cylinder(d=tube_D1, h=100, center=false);
			
			translate([0, 0, mount_H-mount_center_hole_H]) cylinder(d=mount_center_hole_D, h=mount_center_hole_H+10, center=false);
			
			rotate([0, 0, -90]) translate([mount_hole_pos_R, 0, mount_H - mount_layer_H]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			rotate([0, 0, 120 - 90]) translate([mount_hole_pos_R, 0, mount_H - mount_layer_H]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			rotate([0, 0, 240 - 90]) translate([mount_hole_pos_R, 0, mount_H - mount_layer_H]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			
			rotate([0, 0, 0 + A2 + 15]) translate([ball_D/2 - conn_dr, 0, conn_H]) mirror([0, 0, 1]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			rotate([0, 0, 90 + A2 - 15]) translate([ball_D/2 - conn_dr, 0, conn_H]) mirror([0, 0, 1]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			rotate([0, 0, 180 + A2]) translate([ball_D/2 - conn_dr, 0, conn_H]) mirror([0, 0, 1]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			rotate([0, 0, 270 + A2]) translate([ball_D/2 - conn_dr, 0, conn_H]) mirror([0, 0, 1]) hole(mount_hole_D, mount_hole_D2, 40, 40);
			
		}
	}
}


translate([-ball_D/2 - 1, 0, 0]) rotate([180, 0, 0]) mirror([0, 0, 1]) half_mount();

translate([ball_D/2 + 1, 0, 0]) half_mount();

