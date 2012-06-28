--
-- PostgreSQL database dump
--

SET client_encoding = 'UNICODE';
SET check_function_bodies = false;

SET search_path = public, pg_catalog;

--
-- TOC entry 5 (OID 92443)
-- Name: schema_info; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE schema_info (
    "version" integer
);


--
-- TOC entry 6 (OID 101247)
-- Name: users; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE users (
    id serial NOT NULL,
    login character varying(255),
    email character varying(255),
    crypted_password character varying(40),
    salt character varying(40),
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    remember_token character varying(255),
    remember_token_expires_at timestamp without time zone
);


--
-- TOC entry 7 (OID 101258)
-- Name: shows; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE shows (
    id serial NOT NULL,
    name character varying(8) NOT NULL,
    title character varying(120),
    director character varying(120),
    production_date date,
    delivery_date date,
    studio character varying(120),
    distributor character varying(120)
);


--
-- TOC entry 8 (OID 101266)
-- Name: sequences; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE sequences (
    id serial NOT NULL,
    name character varying(4),
    show_id integer NOT NULL,
    description text
);


--
-- TOC entry 9 (OID 101277)
-- Name: shots; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE shots (
    id serial NOT NULL,
    name character varying(8),
    sequence_id integer NOT NULL
);


--
-- TOC entry 10 (OID 101285)
-- Name: icc_profiles; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE icc_profiles (
    id serial NOT NULL,
    name character varying(256),
    filename character varying(1024)
);


--
-- TOC entry 11 (OID 101297)
-- Name: categories; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE categories (
    id serial NOT NULL,
    name character varying(1024) NOT NULL
);


--
-- TOC entry 12 (OID 101308)
-- Name: audios; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE audios (
    id serial NOT NULL,
    directory character varying(1024) NOT NULL,
    filename character varying(256) NOT NULL,
    stream integer,
    image_id integer,
    creator character varying(20),
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    disk_space integer DEFAULT 0,
    date timestamp without time zone NOT NULL,
    shot_id integer,
    codec character varying(30) NOT NULL,
    fourcc character varying(30) NOT NULL,
    channels integer DEFAULT 1 NOT NULL,
    frequency integer NOT NULL,
    bitrate integer,
    "start" double precision NOT NULL,
    duration double precision NOT NULL,
    online boolean DEFAULT true,
    backup character varying(10)
);


--
-- TOC entry 13 (OID 101322)
-- Name: render_transforms; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE render_transforms (
    id serial NOT NULL,
    name character varying(256) NOT NULL
);


--
-- TOC entry 14 (OID 101330)
-- Name: look_mod_transforms; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE look_mod_transforms (
    id serial NOT NULL,
    name character varying(256) NOT NULL
);


--
-- TOC entry 15 (OID 101338)
-- Name: pixel_formats; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE pixel_formats (
    id serial NOT NULL,
    name character varying(256) NOT NULL
);


--
-- TOC entry 16 (OID 101352)
-- Name: images; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE images (
    id serial NOT NULL,
    directory character varying(1024) NOT NULL,
    filename character varying(256) NOT NULL,
    shot_id integer,
    frame_start integer,
    frame_end integer,
    creator character varying(20),
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    width integer NOT NULL,
    height integer NOT NULL,
    pixel_ratio double precision DEFAULT 1.0 NOT NULL,
    date timestamp without time zone NOT NULL,
    format character varying(30),
    fps double precision DEFAULT 24.0,
    codec character varying(10),
    disk_space integer DEFAULT 1048576 NOT NULL,
    icc_profile_id integer,
    render_transform_id integer,
    look_mod_transform_id integer,
    depth integer DEFAULT 32 NOT NULL,
    pixel_format_id integer,
    num_channels integer DEFAULT 4 NOT NULL,
    layers character varying(1024) NOT NULL,
    fstop double precision DEFAULT 8.0,
    gamma double precision DEFAULT 1.0,
    online boolean DEFAULT true NOT NULL,
    rating integer,
    backup character varying(10),
    label_color character varying(6),
    description character varying(1024),
    thumbnail_width integer DEFAULT 0 NOT NULL,
    thumbnail_height integer DEFAULT 0 NOT NULL,
    thumbnail bytea
);


--
-- TOC entry 17 (OID 101373)
-- Name: image_categories; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE image_categories (
    id serial NOT NULL,
    image_id integer,
    category_id integer
);


--
-- TOC entry 18 (OID 101380)
-- Name: videos; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE videos (
    id serial NOT NULL,
    image_id integer NOT NULL,
    stream integer NOT NULL,
    created_at timestamp without time zone NOT NULL,
    updated_at timestamp without time zone NOT NULL,
    codec character varying(30),
    fourcc character varying(30),
    pixel_format character varying(30),
    fps double precision NOT NULL,
    "start" double precision NOT NULL,
    duration double precision NOT NULL
);


--
-- TOC entry 19 (OID 101388)
-- Name: video_categories; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE video_categories (
    id serial NOT NULL,
    video_id integer,
    category_id integer
);


--
-- TOC entry 20 (OID 101395)
-- Name: audio_categories; Type: TABLE; Schema: public; Owner: mrviewer
--

CREATE TABLE audio_categories (
    id serial NOT NULL,
    audio_id integer,
    category_id integer
);


--
-- TOC entry 22 (OID 101263)
-- Name: shows_name_key; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX shows_name_key ON shows USING btree (name);


--
-- TOC entry 25 (OID 101274)
-- Name: sequences_show_id_key; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX sequences_show_id_key ON sequences USING btree (show_id, name);


--
-- TOC entry 27 (OID 101282)
-- Name: shots_sequence_id_key; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX shots_sequence_id_key ON shots USING btree (sequence_id, name);


--
-- TOC entry 29 (OID 101293)
-- Name: icc_profiles_name_key; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX icc_profiles_name_key ON icc_profiles USING btree (name);


--
-- TOC entry 28 (OID 101294)
-- Name: icc_profiles_filename_key; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX icc_profiles_filename_key ON icc_profiles USING btree (filename);


--
-- TOC entry 31 (OID 101305)
-- Name: categories_name_key; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX categories_name_key ON categories USING btree (name);


--
-- TOC entry 33 (OID 101319)
-- Name: audios_idx; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX audios_idx ON audios USING btree (filename, directory, stream);


--
-- TOC entry 35 (OID 101327)
-- Name: render_transforms_idx; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX render_transforms_idx ON render_transforms USING btree (name);


--
-- TOC entry 37 (OID 101335)
-- Name: look_mod_transforms_idx; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX look_mod_transforms_idx ON look_mod_transforms USING btree (name);


--
-- TOC entry 39 (OID 101343)
-- Name: pixel_formats_idx; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX pixel_formats_idx ON pixel_formats USING btree (name);


--
-- TOC entry 41 (OID 101370)
-- Name: images_idx; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX images_idx ON images USING btree (filename, directory);


--
-- TOC entry 44 (OID 101385)
-- Name: videos_idx; Type: INDEX; Schema: public; Owner: mrviewer
--

CREATE UNIQUE INDEX videos_idx ON videos USING btree (image_id, stream);


--
-- TOC entry 21 (OID 101253)
-- Name: users_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);


--
-- TOC entry 23 (OID 101261)
-- Name: shows_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY shows
    ADD CONSTRAINT shows_pkey PRIMARY KEY (id);


--
-- TOC entry 24 (OID 101272)
-- Name: sequences_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY sequences
    ADD CONSTRAINT sequences_pkey PRIMARY KEY (id);


--
-- TOC entry 26 (OID 101280)
-- Name: shots_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY shots
    ADD CONSTRAINT shots_pkey PRIMARY KEY (id);


--
-- TOC entry 30 (OID 101291)
-- Name: icc_profiles_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY icc_profiles
    ADD CONSTRAINT icc_profiles_pkey PRIMARY KEY (id);


--
-- TOC entry 32 (OID 101303)
-- Name: categories_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY categories
    ADD CONSTRAINT categories_pkey PRIMARY KEY (id);


--
-- TOC entry 34 (OID 101317)
-- Name: audios_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY audios
    ADD CONSTRAINT audios_pkey PRIMARY KEY (id);


--
-- TOC entry 36 (OID 101325)
-- Name: render_transforms_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY render_transforms
    ADD CONSTRAINT render_transforms_pkey PRIMARY KEY (id);


--
-- TOC entry 38 (OID 101333)
-- Name: look_mod_transforms_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY look_mod_transforms
    ADD CONSTRAINT look_mod_transforms_pkey PRIMARY KEY (id);


--
-- TOC entry 40 (OID 101341)
-- Name: pixel_formats_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY pixel_formats
    ADD CONSTRAINT pixel_formats_pkey PRIMARY KEY (id);


--
-- TOC entry 42 (OID 101368)
-- Name: images_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY images
    ADD CONSTRAINT images_pkey PRIMARY KEY (id);


--
-- TOC entry 43 (OID 101376)
-- Name: image_categories_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY image_categories
    ADD CONSTRAINT image_categories_pkey PRIMARY KEY (id);


--
-- TOC entry 45 (OID 101383)
-- Name: videos_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY videos
    ADD CONSTRAINT videos_pkey PRIMARY KEY (id);


--
-- TOC entry 46 (OID 101391)
-- Name: video_categories_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY video_categories
    ADD CONSTRAINT video_categories_pkey PRIMARY KEY (id);


--
-- TOC entry 47 (OID 101398)
-- Name: audio_categories_pkey; Type: CONSTRAINT; Schema: public; Owner: mrviewer
--

ALTER TABLE ONLY audio_categories
    ADD CONSTRAINT audio_categories_pkey PRIMARY KEY (id);


--
-- TOC entry 3 (OID 2200)
-- Name: SCHEMA public; Type: COMMENT; Schema: -; Owner: postgres
--

COMMENT ON SCHEMA public IS 'Standard public schema';


INSERT INTO schema_info (version) VALUES (15)