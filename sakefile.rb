# -*- ruby -*-
# sake variant sake4

#
# sakefile.rb
#
# Copyright 2008 Helsinki Institute for Information Technology (HIIT)
# and the authors. All rights reserved.
#
# Authors: Tero Hasu <tero.hasu@hut.fi>
#

# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# The build tool driving this makefile is a custom one. The required
# version has not yet been released.

require 'sake4/component'

def try_load file
  begin
    load file
  rescue LoadError; end
end

$pys60_version = ($sake_op[:pys60] ? $sake_op[:pys60].to_i : 1)

case $pys60_version
when 1
  $uid_v8 = 0x03462346
when 2
  $uid_v8 = 0x03462347
else
  raise "unsupported PyS60 version"
end

load("version")

$basename = "miso"
$version = MISOVER.to_s.split(".").map {|x| x.to_i}

$proj = Sake::Project.new(:basename => $basename,
                          :name => "Miso for PyS60v#{$pys60_version}",
                          :version => $version,
                          # This is a test UID.
                          :uid => Sake::Uid.v8($uid_v8),
                          :vendor => "HIIT")

class <<$proj
  def pkg_in_file
    group_dir + ("module.pkg.in")
  end
end

$pyd = Sake::Component.new(:project => $proj,
                           :target_type => :pyd,
                           :basename => $basename,
                           :bin_basename => $basename,
                           :uid3 => Sake::Uid.v8($uid_v8),
                           :caps => Sake::ALL_CAPS)

class <<$pyd
  def mmp_in_file
    group_dir + ("module.mmp.in")
  end
end

$comp_list = [$pyd].compact

if $sake_op[:kits]
  $kits = Sake::DevKits::get_exact_set($sake_op[:kits].strip.split(/,/))
else
  $kits = Sake::DevKits::get_all
end

$kits.delete_if do |kit|
  !kit.supports_python_v?($pys60_version)
end

if $sake_op[:comps]
  comps = $sake_op[:comps].strip.split(/,/)
  $comp_list.delete_if do |comp|
    !comps.include?(comp.basename)
  end
end

$builds = $kits.map do |kit|
  build = Sake::ProjBuild.new(:project => $proj,
                              :devkit => kit)
  if build.v9_up?
    build.handle = (build.handle + ("_py%d" % $pys60_version))
  end
  build.abld_platform = (build.v9_up? ? "gcce" : "armi")
  build.abld_build = ($sake_op[:udeb] ? "udeb" : "urel")
  if $sake_op[:udeb]
    build.handle = (build.handle + "_udeb")
  end
  if $sake_op[:span_5th]
    build.handle = (build.handle + "_5th")
    $span_s60_5th = true
  end
  if build.v9_up?
    build.gcc_version = ($sake_op[:gcce] ? $sake_op[:gcce].to_i : 3)
    if build.gcc_version > 3
      build.handle = (build.handle + ("_gcce%d" % build.gcc_version))
    end
  end
  build
end

# For any v9 builds, configure certificate info for signing.
try_load('local/signing.rb')

$builds.delete_if do |build|
  (build.sign and !build.cert_file)
end

if $sake_op[:builds]
  blist = $sake_op[:builds]
  $builds.delete_if do |build|
    !blist.include?(build.handle)
  end
end

$builds.sort! do |x,y|
  x.handle <=> y.handle
end

desc "Prints a list of possible builds."
task :builds do
  for build in $builds
    puts build.handle
  end
end

desc "Prints info about possible builds."
task :build_info do
  for build in $builds
    puts "#{build.handle}:"
    puts "  project name  #{build.project.name}"
    puts "    basename    #{build.project.basename}"
    puts "  target        #{build.target.handle}"
    puts "  devkit        #{build.devkit.handle}"
    puts "  abld platform #{build.abld_platform}"
    puts "  abld build    #{build.abld_build}"
    puts "  sign SIS?     #{build.sign}"
    puts "  cert file     #{build.cert_file}"
    puts "  privkey file  #{build.key_file}"
    puts "  cert caps     #{build.max_caps.inspect}"
    puts "  components    #{build.comp_builds.map {|x| x.component.basename}.inspect}"
  end
end

class HexNum
  def initialize num
    @num = num
  end

  def to_s
    "0x%08x" % @num
  end
end

class Sake::ProjBuild
  def needs_pyd_wrapper?
    # Some problems with imp.load_dynamic, better avoid the issue for
    # now, especially as we are not using the wrapper to include pure
    # Python code yet.
    false # v9?
  end
end

class Sake::CompBuild
  def binary_prefix
    return "" if v8_down?
    pfx =
      case $pys60_version
      when 1 then ""
      when 2 then "kf_"
      else raise end
    pfx += "_" if needs_pyd_wrapper?
    pfx
  end

  def binary_suffix
    return "" unless needs_pyd_wrapper?
    "_" + uid3.chex_string
  end

  def binary_file
    generated_file("%s%s%s.%s" % [binary_prefix,
                                  bin_basename,
                                  binary_suffix,
                                  target_ext])
  end

  def pyd_wrapper_basename
    bin_basename
  end

  def pyd_wrapper_file
    generated_file(pyd_wrapper_basename + ".py")
  end

  def pyd_wrapper_path_in_pkg
    # To look for suitable paths, use
    #
    #   import sys
    #   sys.path
    #
    # Yes, it seems putting wrappers on E: is not an option.
    case $pys60_version
    when 1 then "c:\\resource\\"
    when 2 then "c:\\resource\\python25\\"
    else raise end
  end
end

$cbuild_by_pbuild = Hash.new
for pbuild in $builds
  map = pbuild.trait_map

  # To define __UID__ for header files.
  if pbuild.uid
    map[:uid] = HexNum.new(pbuild.uid.number)
  end

  map[:pys60_version] = $pys60_version

  map[($basename + "_version").to_sym] = ($version[0] * 100 + $version[1])

  modname = (pbuild.needs_pyd_wrapper? ? ("_" + $basename) : $basename)
  map[:module_name] = modname
  map[:init_func_name] = ("init" + modname).to_sym

  # NDEBUG controls whether asserts are to be compiled in (NDEBUG is
  # defined in UDEB builds). Normally an assert results in something
  # being printed to the console. To also have the errors logged, you
  # will want to enable logging by setting "logging=true". Without
  # this setting, there will be no dependency on the (deprecated) file
  # logger API, and errors are still displayed on the console (if you
  # have one and have time to read it). "logging=true" has no effect
  # if your SDK does not have the required API.
  if $sake_op[:logging] and map[:has_flogger]
    map[:do_logging] = :define
  end

  # Each build variant shall have all of the components.
  pbuild.comp_builds = $comp_list.map do |comp|
    b = Sake::CompBuild.new(:proj_build => pbuild,
                            :component => comp)
    $cbuild_by_pbuild[pbuild] = b
    b
  end
end

task :default => [:bin, :sis]

require 'sake4/tasks'

Sake::Tasks::def_list_devices_tasks(:builds => $builds)

Sake::Tasks::def_makefile_tasks(:builds => $builds)

Sake::Tasks::def_binary_tasks(:builds => $builds, :pyd_wrapper => true)

Sake::Tasks::def_sis_tasks(:builds => $builds)

Sake::Tasks::def_clean_tasks(:builds => $builds)

# :all and :release together do everything. :all by itself does not
# touch the downloads containing version-numbered files.
task :all => [:makefiles, :bin, :sis]

# We probably do not require separate documentation for every single
# build variant, as at least the interface should be just about the
# same in each file; although given that we are documenting private
# stuff, too, there could be differences, but still, let us just pick
# one version.
$doc_build = $builds.last

if $doc_build
  # C++ API documentation.
  Sake::Tasks::def_doxygen_tasks(:build => $doc_build)
  task :all => :cxxdoc

  # Python API documentation.
  py_file = $doc_build.src_dir + "miso.py"
  Sake::Tasks::def_epydoc_tasks(:build => $doc_build, :py_file => py_file)
  task :all => :pydoc
end

# Configure any rules related to releasing and uploading and such
# things. Probably at least involves copying or uploading the
# distribution files somewhere.
try_load('local/releasing.rb')

dl_dir = $proj.download_dir
dl_path = $proj.to_proj_rel(dl_dir).to_s

task :release_sis do
  mkdir_p dl_path

  for build in $builds
    ## Unsigned.
    if (not build.sign_sis?) or (build.sign_sis? and ($cert_name == "dev"))
      src_sis_file = build.to_proj_rel(build.long_sis_file).to_s
      sis_basename = File.basename(src_sis_file)
      download_file = File.join(dl_path, sis_basename)
      ln(src_sis_file, download_file, :force => true)
    end

    ## Signed.
    if build.sign_sis? and ($cert_name == "self")
      src_sis_file = build.to_proj_rel(build.long_sisx_file).to_s
      sis_basename = File.basename(src_sis_file)
      download_file = File.join(dl_path, sis_basename)
      ln(src_sis_file, download_file, :force => true)
    end
  end
end

TAR_FMT = "tar -c -z -v -f %s '--exclude=*_dev/*.sisx' '--exclude=*dev.sisx' '--exclude=*~' '--exclude=#*' '--exclude=.*' '--exclude=*.pyc' build doxyfile-int Makefile README sakefile.rb src tools web"

desc "Prepares a snapshot of current devel source."
task :snapshot do
  mkdir_p dl_path
  tar_file = dl_dir + ("%s-devel-src.tar.gz" % [$basename])
  sh(TAR_FMT % [tar_file])
end

desc "Prepares a release source snapshot."
task :release_src do
  mkdir_p dl_path
  tar_file = dl_dir + ("%s-%s-src.tar.gz" % [$basename, $proj.version_string])
  sh(TAR_FMT % [tar_file])
end

desc "Prepares downloads for the current version."
task :release => [:release_sis, :release_src] do
  # The .py file is enough. HTML only for most recent release.
  #api_sfile = $proj.python_api_dir + ($pyd.basename + ".html")
  #api_dfile = dl_dir + ("%s-%s-api.html" % [$pyd.basename, $proj.version_string])
  #install api_sfile.to_s, api_dfile.to_s, :mode => 0644

  api_sfile = $proj.src_dir + ($pyd.basename + ".py")
  api_dfile = dl_dir + ("%s-%s-api.py" % [$pyd.basename, $proj.version_string])
  install api_sfile.to_s, api_dfile.to_s, :mode => 0644
end

def sis_info opt
  for build in $builds
    if build.short_sisx_file.exist?
      sh("sisinfo -f #{build.short_sisx_file} #{opt}")
    end
  end
end

Sake::Tasks::force_uncurrent_on_op_change

task :sis_ls do
  sis_info "-i"
end

task :sis_cert do
  sis_info "-c"
end

task :sis_struct do
  sis_info "-s"
end
