class IccProfilesController < ApplicationController
  def index
    list
    render :action => 'list'
  end

  # GETs should be safe (see http://www.w3.org/2001/tag/doc/whenToUseGet.html)
  verify :method => :post, :only => [ :destroy, :create, :update ],
         :redirect_to => { :action => :list }

  def list
    @icc_profile_pages, @icc_profiles = paginate :icc_profiles, :per_page => 10
  end

  def show
    @icc_profile = IccProfile.find(params[:id])
  end

  def new
    @icc_profile = IccProfile.new
  end

  def create
    @icc_profile = IccProfile.new(params[:icc_profile])
    if @icc_profile.save
      flash[:notice] = 'IccProfile was successfully created.'
      redirect_to :action => 'list'
    else
      render :action => 'new'
    end
  end

  def edit
    @icc_profile = IccProfile.find(params[:id])
  end

  def update
    @icc_profile = IccProfile.find(params[:id])
    if @icc_profile.update_attributes(params[:icc_profile])
      flash[:notice] = 'IccProfile was successfully updated.'
      redirect_to :action => 'show', :id => @icc_profile
    else
      render :action => 'edit'
    end
  end

  def destroy
    IccProfile.find(params[:id]).destroy
    redirect_to :action => 'list'
  end
end
