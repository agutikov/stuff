

module arc (r_inner, r_outer, l) {
		intersection() {
			rotate([0, 90, 0])
			difference(){
				cylinder(d=2*r_outer, h=l);
				cylinder(d=2*r_inner, h=l);
			}
			translate([0, -r_outer, -r_outer])
			cube([l, r_outer, r_outer]);
		}
}

/*****************
	Уголок профильный:
	s - толщина полки
	h - одна полка
	w - вторая полка
	l - длинна
	r - внутренний радиус закругления
******************/

module profile_L (s, h, w, l, r) {
	union() {
		translate([0, r+s, r+s])
		arc(r, r+s, l);
		cube([l, s, h]);
		cube([l, w, s]);
	}
}

// profile_L(5, 20, 50, 300, 6);


/***********************
	Труба профильная прямоугольная:
	s - толщина стенки
	h - толщина трубы 
	w - толщина трубы вторая
	l - длинна
	r - внешний радиус закругления
************************/

module profile_pipe (s, h, w, l, r) {
	union () {
		translate([0, r, r])
		arc(r-s, r, l);

		translate([0, w-r, r])
		rotate([90, 0, 0])
		arc(r-s, r, l);

		translate([0, w-r, h-r])
		rotate([180, 0, 0])
		arc(r-s, r, l);

		translate([0, r, h-r])
		rotate([270, 0, 0])
		arc(r-s, r, l);

		translate([0, 0, r])
		cube([l, s, h-2*r]);

		translate([0, w-s, r])
		cube([l, s, h-2*r]);

		translate([0, r, 0])
		cube([l, w-2*r, s]);

		translate([0, r, h-s])
		cube([l, w-2*r, s]);
	}
}

//profile_pipe(3, 50, 30, 300, 4);


/***************************
	Швеллер:
	s - толщина стенок
	w - ширина
	h - высота полок
	l - длина
	r - внутренний радиус
****************************/


module channel (s, w, h, l, r) {
	union () {	
		translate([0, r+s, r+s])
		arc(r, r+s, l);

		translate([0, w-(r+s), r+s])
		rotate([90, 0, 0])
		arc(r, r+s, l);

		translate([0, 0, r])
		cube([l, s, h]);

		translate([0, w-s, r])
		cube([l, s, h]);

		cube([l, w, s]);
	}
}

//channel(3, 50, 30, 300, 3);




/***************************
	Двутавр:
	s - толщина стенок
	h - высота
	w - ширина полок
	l - длина
	r - внутренний радиус
****************************/

module I_beam (s, h, w, l, r) {
	union () {

		translate([0, -s, r+s])
		rotate([90, 0, 0])
		arc(r, r+s, l);

		translate([0, r+s, r+s])
		rotate([0, 0, 0])
		arc(r, r+s, l);


		translate([0, -s, h-r-s])
		rotate([180, 0, 0])
		arc(r, r+s, l);

		translate([0, r+s, h-r-s])
		rotate([270, 0, 0])
		arc(r, r+s, l);

		cube([l, s, h]);
		
		translate([0, -w/2+s/2, h-s])
		cube([l, w, s]);

		translate([0, -w/2+s/2, 0])
		cube([l, w, s]);
	}
}

//I_beam(3, 50, 30, 300, 3);


